/*
 * oggStreamDecoder is a class to extract an ogg packet from an
 * ogg page stream
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
#include <vector>
#include <cstring>

#include "definition.h"
#include "oggHeader.h"
#include "oggStreamDecoder.h"
#include "exception.h"
#include "log.h"

OggStreamDecoder::SegmentElement::SegmentElement(uint8* _data, uint32 length) :
  data(_data), length(length)
{
}

OggStreamDecoder::OggStreamDecoder()
{
}

OggStreamDecoder::~OggStreamDecoder()
{
  clear();
}

void OggStreamDecoder::init(OggPage page)
{

  /* if this is not a Begin Of Stream page, do nothing */
  if (!page->isBOS()) {
    logger.error() << "OggStreamDecoder: ogg page is not a begin of stream\n";
    return;
  }

  m_packetCount = 0;

  /* extract and remember the serial number of this stream */
  m_serialNo = page->serialno();
  setConfigured();

}

void OggStreamDecoder::clear()
{
  delete[] m_tmpSegment.data;
  m_tmpSegment.data = 0;
  m_tmpSegment.length = 0;
}

uint32 OggStreamDecoder::getSerialNo()
{
  return(m_serialNo);
}

OggStreamDecoder& OggStreamDecoder::operator<<(OggPage& page)
{
  /* if this stream is not initialized, try to initialize it */
  if (!isInitialized())
    init(page);

  /* decode the packets */
  if (!isConfigured()) {
    throw OggException("OggStreamDecoder::operator<<: This stream is not is not configured yet");
  }

  if (page->serialno() != m_serialNo) {
    throw OggException("OggStreamDecoder::operator<<: page does not belong to this stream");
  }

  /* extract the header */
  uint8* data(&(page->data())[0]);
  OggHeader* header = (OggHeader*)(data);
  data += sizeof(OggHeader);

  /* extract the relevant data from the header */
  unsigned char tableSegments(header->tableSegments);

  // extract the segment table
  uint8* segment = (uint8*) data;
  data += tableSegments;

  /* will the last packet be continued on in the next page? */
  bool willBeContinued;

  if (segment[header->tableSegments-1] != 255)
    willBeContinued = false;
  else
    willBeContinued = true;

  std::vector<SegmentElement> segmentDataList;

  // extract pointers to the packets in this page
  SegmentElement segData(data,0);

  for (unsigned int i=0; i<tableSegments; ++i) {
    data += segment[i];
    segData.length += segment[i];
    if (segment[i] != 255) {
      segmentDataList.push_back(segData);
      segData = SegmentElement(data,0);
    }
  }

  // store the last packet if it does not end in this page
  if (willBeContinued)
    segmentDataList.push_back(segData);

  /* it would be good to know where the granule position belongs to */
  uint32 infoPosition(256);
  /* does the last packet do not end here */
  if (!willBeContinued)
    infoPosition = segmentDataList.size()-1;
  else if (segmentDataList.size() > 1)
    infoPosition = segmentDataList.size()-2;

  // now extract the ogg packets itself
  // every segment in the list is one packet (maybe there is a
  // remaining part in tmpSegment from the page before and
  // there might be a segment, that is not finished on this page)

  for (unsigned int i(0); i<segmentDataList.size(); ++i) {

    uint32 overallLength = m_tmpSegment.length+segmentDataList[i].length;
    unsigned char* newPacketPtr = new unsigned char[overallLength];

    if (m_tmpSegment.length)
      memcpy(newPacketPtr, m_tmpSegment.data, m_tmpSegment.length);

    memcpy(newPacketPtr+m_tmpSegment.length, segmentDataList[i].data, segmentDataList[i].length);

    // delete the temporary Segment
    if (m_tmpSegment.data) {
      delete[] m_tmpSegment.data;
      m_tmpSegment = SegmentElement();
    }
    if ((i == (segmentDataList.size()-1)) && willBeContinued) {
      //store last segment as it is does not end here
      m_tmpSegment = SegmentElement(newPacketPtr, overallLength);
    } else {
      // we found a full packet
      OggPacketInternal::PacketType packetType(OggPacketInternal::PacketType::normal);
      int64 granulePosition(-1);

      if ((i == 0) && (page->isBOS()))
        packetType = OggPacketInternal::PacketType::bos;

      if ((i == segmentDataList.size()-1) && (page->isEOS()))
        packetType = OggPacketInternal::PacketType::eos;

      if (i == infoPosition)
        granulePosition = header->position;

      /* create the packet */
      OggPacket packet(new OggPacketInternal(newPacketPtr, overallLength, m_packetCount++, granulePosition, packetType));
      delete[] newPacketPtr;

      m_oggPacketList.push_back(packet);
    }
  }

  if (!m_oggPacketList.empty())
    setAvailable();

  return(*this);
}

OggPacket OggStreamDecoder::inspectNextPacket()
{
  OggPacket packet;

  if (!isAvailable()) {
    throw OggException("OggStreamDecoder::inspectNextPacket: no packet available");
  }

  // we will not harm the list in any kind
  packet = m_oggPacketList.front();

  return (packet);
}

OggStreamDecoder& OggStreamDecoder::operator>>(OggPacket& packet)
{
  if (!isAvailable()) {
    throw OggException("OggStreamDecoder::operator>>: no packet available");
  }

  packet = m_oggPacketList.front();
  m_oggPacketList.pop_front();

  /* is this the last packet within this stream,
   * then set the stream status */
  if (packet->isEOS()) {
    setEndOfStream();
  } else {
    if (m_oggPacketList.empty()) {
      setEmpty();
    }
  }
  return(*this);
}

