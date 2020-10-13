/*
 * streamMux will multiplex a number streams to one ogg file
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

#include "streamMux.h"

#include "rawMediaPacket.h"
#include "oggBOSExtractorFactory.h"
#include "log.h"

MuxStreamEntry::MuxStreamEntry()
  : used(false), streamEncoder(0), posInterpreter(0),
    nextTime(-2), empty(true), bufferElemCounter(0)
{
}

MuxStreamEntry::MuxStreamEntry(StreamConfig& config,
                               OggStreamEncoder* _streamEncoder,
                               GranulePosInterpreter* _posInterpreter)
  : used(true), streamConfig(config), streamEncoder(_streamEncoder),
    posInterpreter(_posInterpreter), nextTime(-2), empty(true), lastPacketNo(2),
    bufferElemCounter(0)
{
}

MuxStreamEntry::~MuxStreamEntry()
{
}

StreamMux::StreamMux(MediaRepository* _repository)
  : m_timeOfLastPage(0.0), m_redoTiming(true), m_repository(_repository)
{
}

StreamMux::~StreamMux()
{
  for (uint32 i(0); i<m_streamList.size(); ++i) {
    delete m_streamList[i].streamEncoder;
    delete m_streamList[i].posInterpreter;
  }
  close();

}

void StreamMux::recreatePacketPosition(bool _redoTiming)
{
  m_redoTiming = _redoTiming;
}

void StreamMux::writeToRepository()
{
  RawMediaPacket rawPacket;

  // write the data bunches to the m_repository
  while (m_oggEncoder.isAvailable()) {
    m_oggEncoder    >> rawPacket;
    (*m_repository) << rawPacket;
  }

}

void StreamMux::insertHeader()
{
  OggPage oggPage;

  /* first set all BOS Packets/Pages */
  for (uint32 i(0); i<m_streamList.size(); ++i) {

    if (!m_streamList[i].used)
      continue;

    // easier access
    OggStreamEncoder& encoder = *m_streamList[i].streamEncoder;

    //logger.info() << "inserting first header packet: \n" << m_streamList[i].streamConfig.headerList[0]->toString(10);

    encoder << m_streamList[i].streamConfig.headerList[0];

    // we do not have to flush, the encoder knows, that the bos
    // page needs a clean page

    if (encoder.isAvailable()) {
      encoder    >> oggPage;
      m_oggEncoder << oggPage;

    }
  }

  /* then set the rest of the packets */
  for (uint32 i(0); i<m_streamList.size(); ++i) {

    /* if there are no additional header available, do nothing */
    if (m_streamList[i].streamConfig.headerList.size() <= 1)
      continue;

    // easier access
    OggStreamEncoder& encoder = *m_streamList[i].streamEncoder;

    // insert the rest of the header files
    for (uint32 j(1); j<m_streamList[i].streamConfig.headerList.size(); ++j) {
//      logger.info() << "inserting first header packet: \n" << m_streamList[i].streamConfig.headerList[j]->toString(3);
      encoder << m_streamList[i].streamConfig.headerList[j];
    }

    // place the additional header packets on a clean page
    encoder.flush();

    // write all pages to the ogg encoder
    while (encoder.isAvailable()) {
      encoder    >> oggPage;
      m_oggEncoder << oggPage;
    }

    m_streamList[i].lastPacketNo = m_streamList[i].streamConfig.headerList.size()-1;
  }

  // write the data to the m_repository
  writeToRepository();

}

void StreamMux::writeToOggEncoder()
{
  // write data to the ogg encoder, as long as there is
  // at least one packet of every stream available
  // This is because we might not know the end of stream

  while (!m_outputPageList.empty()) {

    // get the first page
    OggPage nextPage = m_outputPageList.back().page;

    // logger.info() << nextPage->toString(5);

    uint8 streamNo(nextPage->getStreamNo());

    // this is the real return reason, the Page list
    // should never be m_empty
    if (m_streamList.at(streamNo).bufferElemCounter < 2)
      break;

    // insert the packet into the ogg encoder
    m_oggEncoder << nextPage;

    // decrement the marker for the elements of this stream
    m_streamList.at(streamNo).bufferElemCounter--;
    m_outputPageList.pop_back();

  }

  writeToRepository();
}

void StreamMux::flushOggEncoder()
{
  // now we search for the last page of every stream
  // and mark it as the "End of Stream"

  while (!m_outputPageList.empty()) {

    // get the first page
    OggPage nextPage = m_outputPageList.back().page;

    uint8 streamNo(nextPage->getStreamNo());

    // is this the last page of this stream, then mark it
    if (m_streamList.at(streamNo).bufferElemCounter == 1) {
      nextPage->setEOS();
      nextPage->createCRC();
    }

    // insert the packet into the ogg encoder
    m_oggEncoder << nextPage;

    // decrement the marker for the elements of this stream
    m_streamList.at(streamNo).bufferElemCounter--;

    m_outputPageList.pop_back();
  }

  // flush the encoder (actually does nothing)
  m_oggEncoder.flush();

}

bool StreamMux::allBuffersEmpty()
{
  // run through the list and find out, if there is
  // a filled buffer

  for (uint32 i(0); i<m_streamList.size(); ++i)
    if (!m_streamList[i].empty)
      return(false);

  return(true);
}

bool StreamMux::findAndInsertNextPage()
{
  double nextTime(-2);
  uint8  nextID(0);

  for (uint32 i(0); i<m_streamList.size(); ++i) {

    if (m_streamList[i].empty)
      continue;

    if ((nextTime < -1) || (m_streamList[i].nextTime < nextTime)) {
      nextTime = m_streamList[i].nextTime;
      nextID   = i;
    }
  }

  // if we have not found any packet, do nothing
  if (nextTime < -1)
    return(false);

  // insert the found page into the page list
  OutputElement elem(m_streamList[nextID].nextPage, m_streamList[nextID].nextTime);

  std::list<OutputElement>::iterator iter(m_outputPageList.begin());

  // -1 is a problem!
  if (elem.time < 0) {
    elem.time = m_timeOfLastPage; // should we care for the stream No?
  }

  while ((iter != m_outputPageList.end()) && (elem.time < iter->time))
    ++iter;

  m_outputPageList.insert(iter, elem);

  m_timeOfLastPage = elem.time;

  // page has been added to the output list, mark the entry as free
  m_streamList[nextID].empty = true;

  // increment the number of elements in the, that
  // belongs to this stream
  // we need to know that, to be able to ensure that at least
  // one packet of every stream is not written to the ogg encoder
  m_streamList[nextID].bufferElemCounter++;

  // write the data
  writeToOggEncoder();

  return(true);
}

void StreamMux::handleNextPage(OggPage& page, uint8 streamNo)
{
  /* here we look, if there is a page in the buffer
   * if yes, look into the other buffers and insert the pages into
   * the m_outputPageList by their time order until the page buffer,
   * we are looking for is free */

  page->setStreamNo(streamNo);
  MuxStreamEntry& entry = m_streamList.at(streamNo);

  while (!entry.empty) {
    findAndInsertNextPage();
  }

  entry.nextPage = page;
  entry.empty    = false;
  entry.nextTime = entry.posInterpreter->getTime(page->granulepos());
  return;

}

void StreamMux::configureStreams(std::vector<StreamConfig>& config)
{
  m_streamList.resize(config.size());

  for (uint32 i(0); i<config.size(); ++i) {

    if (config[i].type == OggType::unknown) {
      continue;
    }

    // create the encoder and the position interpreter
    OggStreamEncoder*      streamEncoder  = new OggStreamEncoder;
    GranulePosInterpreter* posInterpreter =
      OggBOSExtractorFactory::extractPositionInterpreter(config[i]);

    MuxStreamEntry entry(config[i], streamEncoder, posInterpreter);

    m_streamList[config[i].streamNo] = entry;

  }

  // write the header pages (they are available within the StreamConfig information)
  insertHeader();
}

StreamMux& StreamMux::operator<<(OggPacket& packet)
{

  // easier access
  MuxStreamEntry& entry     = m_streamList.at(packet->getStreamNo());

  // can we handle this packet - if not, this packet will be discarded
  if (!entry.used) {
    logger.error() << "StreamMux::operator<< no valid stream to put a packet with stream no:"<<(uint32) packet->getStreamNo()<<std::endl;
    return(*this);
  }

  OggStreamEncoder& encoder = *entry.streamEncoder;

  // first we set a valid granule position to the packet
  if (m_redoTiming)
    entry.posInterpreter->setStreamPosition(packet);

  // then we place this packet into the stream encoder
  encoder << packet;

  // now it is becomming difficult, as the pages must be in the correct order
  // therefor we need a lot of buffering stuff :-/
  OggPage page;

  // if there are one or more pages, handle the buffering
  while (encoder.isAvailable()) {
    encoder >> page;
    handleNextPage(page, packet->getStreamNo());
  }

  return(*this);
}

void StreamMux::setEndOfStream()
{
  // flush the rest, if there is some
  for (uint32 i(0); i<m_streamList.size(); ++i) {

    MuxStreamEntry& entry = m_streamList[i];

    if (!entry.used)
      continue;

    entry.streamEncoder->flush();

    while (entry.streamEncoder->isAvailable()) {
      OggPage page;

      *entry.streamEncoder >> page;
      handleNextPage(page, i);
    }
  }

  // insert all buffered pages
  while (findAndInsertNextPage());

  // flush the page list
  flushOggEncoder();

  writeToRepository();
}

void StreamMux::close()
{
  if (m_repository) {
    m_repository->close();
    delete m_repository;
    m_repository = 0;
  }
}
