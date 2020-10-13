/*
 * oggStreamEncoder is a class to insert an ogg packet into an ogg page stream
 *
 * Copyright (C) 2008 Joern Seger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <iostream>
#include <cstring>
#include <cstdlib>

#include "definition.h"
#include "oggStreamEncoder.h"
#include "oggHeader.h"
#include "crc.h"
#include "exception.h"
#include "log.h"

#define min(a,b) ((a<b)?(a):(b))
#define max(a,b) ((a>b)?(a):(b))

std::vector<uint32> OggStreamEncoder::m_usedSerialNo;

OggStreamEncoder::OggStreamEncoder(uint32 serial)
  : m_maxPageSize(4096), m_streamNo(0), m_segmentsBuffer(maxSegmentEntries), m_dataLength(0), m_dataSegments(0), m_usedData(0), m_pageCounter(0)
//   packetCounter(0), positionInterpreterEnabled(false), pageKeepEnabled(false),
  //posInterpreter(0)
{
  uint32 newSerial = findUniqueSerial(serial);
  m_streamSerialNo = newSerial;
  setInitialized();
}

OggStreamEncoder::~OggStreamEncoder()
{
  if (!m_oggPacketList.empty())
    logger.warning() << "OggStreamEncoder::Destructor: WARNING packet list not m_empty ("<<m_oggPacketList.size()<<" Elements)\n";

  if (!m_oggPageList.empty())
    logger.warning() << "OggStreamEncoder::Destructor: WARNING page list not m_empty\n";

}

// we need a global m_repository to keep an Eye on the serial numbers
uint32 OggStreamEncoder::findUniqueSerial(uint32 origSerial)
{
  bool isUnique(false);
  uint32 serial;

  while (!isUnique) {
    serial     = (origSerial?origSerial:rand());
    isUnique   = true;
    origSerial = 0;

    for (uint32 i(0); i<m_usedSerialNo.size(); ++i)
      if (serial == m_usedSerialNo[i])
        isUnique = false;
  }
  m_usedSerialNo.push_back(serial);
  return(serial);
}

/*
void OggStreamEncoder::enablePositionInterpreter()
{
  positionInterpreterEnabled = true;
}

void OggStreamEncoder::keepOnePage()
{
  pageKeepEnabled = true;
}
*/

void OggStreamEncoder::addPacket(OggPacket& packet)
{
  /* if we want to interprete the position by ourself */
  /*
    if (positionInterpreterEnabled &&
        posInterpreter && (!packet.isStreamHeader()))
      posInterpreter->setStreamPosition(packet);
  */
  /* This is a normal packet
   * Let's start to calculate the actual length */
  m_oggPacketList.push_back(packet);

  m_dataLength             += packet->length();
  uint32 actSegmentsSize  = (packet->length()+255)/255;
  std::vector<uint8_t> actSegments(actSegmentsSize/*maxSegmentEntries*/, 0xff);
  //uint8  actSegments[maxSegmentEntries];

  if (actSegmentsSize > maxSegmentEntries)
    throw OggException("OggStreamEncoder::addPacket: Not able to handle this packet size");

  /* calculate the segment table part of this packet */
//  memset(actSegments, 0xff, actSegmentsSize-1);
  actSegments[actSegmentsSize-1] = packet->length()%255;

  m_segmentsBuffer.insert(m_segmentsBuffer.end(), actSegments.begin(), actSegments.end());

}

bool OggStreamEncoder::getNextPacketLength(uint32 pageBorder, uint32& length,
    uint32& segments)
{
  /* initialize the values */
  length   = 0;
  segments = 0;

  /* if the data length is in range, do nothing */
  if ((m_dataLength < pageBorder) && (m_segmentsBuffer.size() < 255)) {
    return false;
  }

  uint32_t allSegLength=0;
  for(uint32_t tmpLen : m_segmentsBuffer) {
    allSegLength += tmpLen;
  }
  //logger.info() << "\nsegments: "<< m_segmentsBuffer.size()<< "len: "<<allSegLength<< " data length: "<< m_dataLength<<"\n";


  /* and calculate, how many segments we want to include into the
   * actual page */
  bool end_found(false);
  for (uint32_t i(0); i<m_segmentsBuffer.size(); ++i) {
    length  += m_segmentsBuffer[i];
    segments++;
    if ((length >= pageBorder) || (segments >= 254)) {
      if ((i+1 == m_segmentsBuffer.size()) || (m_segmentsBuffer[i+1] != 0)) { // this is due to an empty (0) segment
        break;
      }
    }
  }

  return true;

}

OggStreamEncoder& OggStreamEncoder::operator<<(OggPacket packet)
{
  if (!isInitialized()) {
    logger.error() << "OggStreamEncoder::operator<<: Stream is not initialized correctly\n";
    return(*this);
  }

  if (!isConfigured()) {

    // this must be the bos packet
    if (!packet->isBOS()) {
      logger.error() << "OggStreamEncoder::operator<<: First packet must be a BOS packet\n";
      return(*this);
    } else {
      // The first BOS packet defines the stream number
      m_streamNo = packet->getStreamNo();
    }
    /* add the packet to the temporal buffer */
    addPacket(packet);

    /* the encoder stream is configured */
    setConfigured();

    /* flush the first packet (see Spec) */
    flush();

    return(*this);
  }

  /* add the packet to the temporal buffer */
  addPacket(packet);

  /* create as many packets as data is available */
  while ((m_dataLength >= m_maxPageSize) || (m_segmentsBuffer.size() >= 255)) {
    createPage(m_maxPageSize);
  }

  return(*this);
}

OggStreamEncoder& OggStreamEncoder::operator>>(OggPage& page)
{
  if (isEmpty()) {
    logger.error() << "OggStreamEncoder::opertator>>: no page available\n";
    return(*this);
  }

  page = m_oggPageList.front();
  m_oggPageList.pop_front();

  page->setStreamNo(m_streamNo);

  if (m_oggPageList.empty())
    setEmpty();


  return(*this);

}

void OggStreamEncoder::flush()
{
  while (m_dataLength || (m_segmentsBuffer.size() > 0)) {
    createPage(m_dataLength);
  }
}

void OggStreamEncoder::createPage(uint32 minPageSize)
{
  uint32 bodyLength;
  uint32 segmentsSize;

  /* Is there enough data available? */
  if (!getNextPacketLength(minPageSize, bodyLength, segmentsSize)) {
    return;
  }

  uint32 overallLength = sizeof(OggHeader) + segmentsSize + bodyLength;
  std::vector<uint8_t> pageData(overallLength);
  //uint8* pageData = new uint8[overallLength];

  /* an ogg page looks like this:
   * --------------------------------------------------------
   * | Ogg Header | Segments Table | Packet1 | Packet2 | ...
   * --------------------------------------------------------
   */
  uint8_t* pageBeginPtr = &pageData[0];
  OggHeader* header   ((OggHeader*)pageBeginPtr);
  uint8*     segments (pageBeginPtr+sizeof(OggHeader));
  uint8*     body     (segments+segmentsSize);

  /* set the header information */
  memset(header, 0x00, sizeof(OggHeader));

  memcpy(header->ogg,"OggS",4);
  header->tableSegments = segmentsSize;
  header->pageNo        = m_pageCounter++;
  header->serial        = m_streamSerialNo;
  header->position      = -1;

  if (m_usedData)
    header->pack_type   = 1; // packet starts on the previous page

  /* fill the segments */
  for (uint32_t i(0); i<segmentsSize; ++i)
    segments[i] = m_segmentsBuffer[i];

  m_segmentsBuffer.erase_begin(segmentsSize);

  std::list<OggPacket>::iterator it(m_oggPacketList.begin());

  /* if this is a bos packet, this would surely be the first packet */
  if ((*it)->isBOS())
    header->page_type = 1;

  uint32 arrayIndex(0);
  logger.debug() << "OggStreamEncoder::create_page: packet length="<< overallLength <<"\n";

  for (; it != m_oggPacketList.end(); it++) {

    OggPacket pkt(*it);
    uint32 packetLength = pkt->length();
    uint32 cpyLength = min((packetLength-m_usedData),(bodyLength-arrayIndex));

    logger.debug() << "OggStreamEncoder::create_page: copyLength="<<cpyLength<<" arrayIndex="<<arrayIndex<<" m_usedData="<<m_usedData<<"\n";

    memcpy(body+arrayIndex, pkt->data()+m_usedData, cpyLength);
    arrayIndex += cpyLength;

    /* is this the end of the actual page */
    if (arrayIndex == bodyLength) {

      /* the packet does not start within this page and does not end
       * on this page? */
      if (m_usedData != 0) {
        m_usedData += cpyLength;
        if (m_usedData == packetLength) {
          m_usedData = 0;
          if (pkt->isEOS())
            header->last = 1;

          /* the packet is fully used, so point to the next valid packet */
          ++it; // this might be the .end() sign, however, how cares

        }

      } else {

        if (cpyLength == packetLength) {

          /* this packet ended on this page */
          m_usedData = 0;

          /* at lease the actual Packet has been completed on this page */
//          onePacketCompleted = true;

          /* Is it the end of a stream? Then mark it as such */
          if (pkt->isEOS())
            header->last = 1;

          /* the packet is fully used, so point to the next valid packet */
          ++it; // this might be the .end() sign, however, how cares

        } else {
          /* keep the number of copied bytes for the next page */
          m_usedData = cpyLength;
        }
      }

      // we found the end of the page
      if (m_usedData || it == m_oggPacketList.end())
        break;
    }

    m_usedData = 0;
  }

#ifdef DEBUG
  if ( arrayIndex != m_bodyLength ) {
    logger.error() << "ERROR: array not matching ( index "<<arrayIndex <<" bodylength "<<m_bodyLength << ")\n";
    logger.error() << "SegmentsSize: "<<(int)segmentsSize<< " still available: "<<(int)m_segmentsBuffer.getUsed()<<"\n";
    for (uint32 i(0); i<segmentsSize; ++i)
      logger.error() << " 0x"<<(int)segments[i];
    logger.error() << "\n";
    abort();
  }
#endif

  /* set the granule position if at least one packet has ended on this page
   * The position is taken from the last full packet
   */
  if (m_oggPacketList.begin() != it) {
    std::list<OggPacket>::iterator it1 = it;
    it1--;
    header->position = (*it1)->granulepos();
    m_oggPacketList.erase(m_oggPacketList.begin(), it);
  }

  header->checksum = Crc::create(pageBeginPtr, overallLength);

  OggPage page = std::make_shared<OggPageInternal>(pageData, sizeof(OggHeader)+segmentsSize, bodyLength);
  m_oggPageList.push_back(page);

  m_dataLength -= bodyLength;

  setAvailable();

}


