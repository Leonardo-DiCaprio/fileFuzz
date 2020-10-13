/*
 * streamSerialize will output a serialized stream of packets from a file
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

#include "streamSerializer.h"

#include "fileRepository.h"
#include "oggBOSExtractorFactory.h"
#include "log.h"

StreamEntry::StreamEntry() :
  streamDecoder(0), posInterpreter(0), nextTime(-1), endOfStream(false),
  empty(true)
{
}

StreamEntry::StreamEntry(StreamConfig& config, OggStreamDecoder* sDecoder) :
  streamConfig(config), streamDecoder(sDecoder), posInterpreter(0), nextTime(-1),
  endOfStream(false), empty(true)
{
}

StreamEntry::~StreamEntry()
{
}

bool StreamEntry::allHeadersCollected()
{
  return(streamConfig.numOfHeaderPackets == streamConfig.headerList.size());
}

StreamSerializer::StreamSerializer() :
  initState(created), repository(0), oggDecoder(new OggDecoder),
  streamEndCounter(0)
{
}

StreamSerializer::~StreamSerializer()
{
  close();
}

bool StreamSerializer::open(std::string& datasource)
{

  // actually only file
  repository = new FileRepository(datasource, MediaUnit::read);

  /* has there been a problem with opening the file */
  if (!repository->isAvailable())
    return (false);

  initState = reposOpened;

  // extract the streams
  bool retValue = extractStreams();

  // fill one packet to every stream item
  std::map<uint32, StreamEntry>::iterator it(streamList.begin());
  for (; it != streamList.end(); ++it) {
    StreamEntry& entry = it->second;
    fillStreams();
    insertNextPacket(entry);
  }

  return (retValue);
}

bool StreamSerializer::open(MediaRepository* _repository)
{
  // actually only file
  repository = _repository;

  /* has there been a problem with opening the file */
  if (!repository->isAvailable())
    return (false);

  initState = reposOpened;

  bool retValue = extractStreams();

  // fill one packet to every stream item
  std::map<uint32, StreamEntry>::iterator it(streamList.begin());
  for (; it != streamList.end(); ++it) {
    StreamEntry& entry = it->second;
    fillStreams();
    insertNextPacket(entry);
  }

  return (retValue);
}

bool StreamSerializer::extractStreams()
{

  RawMediaPacket rawPacket;
  OggPage oggPage;
  OggPacket oggPacket;

  int8 streamCounter = 0;

  while (repository->isAvailable()) {

    /* extract a raw data bunch from the file and place it into
     the ogg decoder */
    (*repository) >> rawPacket;
    (*oggDecoder) << rawPacket;

    /* if there is a complete ogg page available, grab it */
    while (oggDecoder->isAvailable()) {

      (*oggDecoder) >> oggPage;

      /* what ID has this page / to what stream does this page belong to */
      uint32 serialID = oggPage->serialno();

      /* if this is a "begin of stream" packet,
       * create a new stream decoder instance */
      if (oggPage->isBOS()) {

        StreamEntry entry;

        /* get all the relevant information from the stream */
        OggBOSExtractorFactory::extractInformation(oggPage, entry.streamConfig);
        entry.streamConfig.streamNo = streamCounter++;

        /* create the stream encoder */
        entry.streamDecoder = new OggStreamDecoder;
        entry.posInterpreter
          = OggBOSExtractorFactory::extractPositionInterpreter(entry.streamConfig);

        streamList[serialID] = entry;

        // insert the first page
        *(streamList[serialID].streamDecoder) << oggPage;

      } else {
        // insert the next page

        OggPacket         oggPacket;
        StreamEntry&      entry         = streamList[serialID];
        OggStreamDecoder& streamDecoder = *(entry.streamDecoder);

        streamDecoder << oggPage;

        /* as long as we need headers and there are packets available
         * fill the header list */
        while ((!entry.allHeadersCollected()) &&
               (entry.streamDecoder->isAvailable())) {

          /* if the list of header packets is not completed, add
           * the next packet to the list */
          streamDecoder >> oggPacket;
          entry.streamConfig.headerList.push_back(oggPacket);
        }

        /* find out, if all header packets have been found */

        bool allStreamsReady(true);
        std::map<uint32, StreamEntry>::iterator it(streamList.begin());
        for (; it != streamList.end(); ++it) {
          if (!it->second.allHeadersCollected()) {
            allStreamsReady = false;
            break;
          }
        }

        if (allStreamsReady)
          return (true);
      }
    }
  }

  logger.error()
      << "StreamSerializer::extractStreams(): extracter was not able to grab all stream header\n";
  return (false);
}

void StreamSerializer::getStreamConfig(std::vector<StreamConfig>& packetList)
{

  std::map<uint32, StreamEntry>::iterator it(streamList.begin());

  fillStreams();

  // it is a bit difficult, we need the original folge
  packetList.resize(streamList.size());

  for (; it != streamList.end(); ++it) {
    StreamEntry& entry = it->second;
    packetList[entry.streamConfig.streamNo] = entry.streamConfig;
  }

}

void StreamSerializer::close()
{

  delete oggDecoder;
  oggDecoder = 0;

  /* close the m_repository */
  if (repository) {
    repository->close();
    delete repository;
    repository = 0;
  }

  std::map<uint32, StreamEntry>::iterator it = streamList.begin();

  /* delete all list entries */
  for (; it != streamList.end(); ++it) {
    StreamEntry entry = it->second;

    delete entry.streamDecoder;
    delete entry.posInterpreter;

//    if (entry.streamConfig.parameter)
//      delete entry.streamConfig.parameter;

    /* we do not need to delete the header List
     * it is controled by the refObject structure */
  }
  streamList.clear();

}

bool StreamSerializer::fillPage()
{

  RawMediaPacket rawPacket;
  OggPage        oggPage;

  while (1==1) {

    // is there no packet available within the ogg page decoder
    while (!oggDecoder->isAvailable()) {

      // is there any data bunch available from the m_repository?
      if (!repository->isAvailable()) {

        // if there is no more data at the m_repository, there is an error
        // in the stream/file
        return (false);
      }

      // get a bunch of raw data and place it into the ogg page decoder
      *repository >> rawPacket;
      *oggDecoder << rawPacket;

      // repeat this until there is at least one page available
    }

    // get the next ogg page
    *oggDecoder >> oggPage;

    // find out to what stream this packet belongs and forget the
    // page if the stream has not been configured befor
    if (streamList.find(oggPage->serialno()) == streamList.end())
      continue;

    // get the stream item for easier access
    StreamEntry& item = streamList[oggPage->serialno()];

    // insert the ogg page into the right stream decoder
    *(item.streamDecoder) << oggPage;

    return (true);
  }

}

/* method is called to be sure, that there is at least one packet in every stream
 * or the stream has finished */
bool StreamSerializer::fillStreams()
{

  /* are there no more packets to process, return false */
  if (streamEndCounter == streamList.size())
    return (false);

  std::map<uint32, StreamEntry>::iterator it = streamList.begin();

  // ensure that every stream can deliver at least one packet or
  // the stream has been ended
  for (; it != streamList.end(); ++it) {

    // create a local reference for easier access
    StreamEntry& item = it->second;

    // if this stream has ended, do not fill this stream any more
    if (item.endOfStream)
      continue;

    // if there is no packet available within this particular stream
    // try to get more input
    while (!item.streamDecoder->isAvailable()) {

      // if the stream has not ended, fill up the stream
      // if the stream has ended, increment the end counter
      if (item.streamDecoder->isEndOfStream()) {
        break;
      } else {
        if (!fillPage()) {
          logger.error() << "StreamSerializer::fillStreams: stream ended without an end-of-stream indicator\n";
          return (false);
        }
      }
    }
  }

  return (true);
}

void StreamSerializer::insertNextPacket(StreamEntry& entry)
{
  // insert next packet into the streamEntry
  if (entry.streamDecoder->isEndOfStream()) {
    // if the stream has ended, set a marker
    if (entry.endOfStream == false) {
      entry.endOfStream = true;
      entry.empty = true;
//      logger.debug() << "Stream <"<<std::hex << entry.streamDecoder->getSerialNo()<< std::dec<<"> has ended \n";
      streamEndCounter++;
    }
//    entry.endOfStream = true;
  } else {

    // get the next packet from this stream decoder
    OggPacket newPacket;
    *(entry.streamDecoder) >> newPacket;

    // set some additional data
    newPacket->setStreamType(entry.streamConfig.type);
    newPacket->setStreamNo(entry.streamConfig.streamNo);

    // if there is a position interpreter, use it to set the time
    // else set the time to 0
    if (entry.posInterpreter) {
//      logger.debug() << "granpos stream: "<<newPacket->granulepos();
      if (newPacket->granulepos() == -1) {
        entry.posInterpreter->setStreamPosition(newPacket);
        entry.nextTime   = entry.posInterpreter->getActTime();
      } else {
        int64 grPos = newPacket->granulepos();
        // the interpreter needs to be pushed forward
        entry.posInterpreter->setStreamPosition(newPacket);
        newPacket->setGranulepos(grPos);
        entry.nextTime = entry.posInterpreter->getTime(newPacket->granulepos());
      }
//      logger.debug() << " calc: "<<newPacket->granulepos()<<std::endl;
    } else {
      entry.nextTime   = 0;
    }

    // set the new packet
    entry.nextPacket = newPacket;
    entry.empty = false;

  }

}

bool StreamSerializer::available()
{

  /* are there no more packets to process, return false */
  if (streamEndCounter == streamList.size())
    return (false);

  return(true);
}

double StreamSerializer::getNextPacket(OggPacket& packet)
{

  /* we need to know, which packet is the next
   * to archive this, we are going to call the next packet of every
   * stream and interpret the granule position, to get
   * the time in seconds, so we are able to compare these positions */

  double time(-1);
  uint32 nextStreamID = 0;

  std::map<uint32, StreamEntry>::iterator it = streamList.begin();

  /* delete all list entries */
  for (; it != streamList.end(); ++it) {

    StreamEntry& entry = it->second;

    /* if this stream has ended, continue with the next stream */
    if (entry.endOfStream)
      continue;

    /* if this is the first packet in this round, take it
     * as a reference else compare both times */

    if ((time < 0) || ((!entry.empty) && (entry.nextTime < time))) {
      time = entry.nextTime;
      nextStreamID = it->first;
    }
  }

  if (time > -1) {
    /* copy the next packet to the requested one */
    packet = streamList[nextStreamID].nextPacket;
    if (fillStreams())
      insertNextPacket(streamList[nextStreamID]);
    else
      streamEndCounter = streamList.size();
  }

  return (time);
}
