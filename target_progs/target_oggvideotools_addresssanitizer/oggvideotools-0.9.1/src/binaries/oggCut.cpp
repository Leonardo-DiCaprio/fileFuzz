/*
 * oggCut is a command line tool, to cut a video stream
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

#ifdef __WIN32
#define __GNU_LIBRARY__
#include "../win32/getopt_win.h"
#endif

#include <iostream>
#include <sstream>
#include <map>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

#include "fileRepository.h"
#include "streamSerializer.h"
#include "streamMux.h"
#include "oggEncoder.h"
#include "oggStreamEncoder.h"
#include "exception.h"
#include "log.h"

struct ListElement {
  double time;
  OggPacket packet;
  ListElement(double _time, OggPacket _packet) :
    time(_time), packet(_packet) {
  }
};

static std::list<ListElement> packetList;
static double bufferTime(0.5); // buffer 500 ms

uint32 _atoi(const char* data)
{
  std::stringstream stream;
  uint32 value;

  stream << data;
  stream >> value;

  return(value);
}

void printHelpScreen(const std::string& progName)
{
  logger.error() << "usage: "<< progName << "[options] <input.ogv> <output.ogv> \n"
                 <<        "or   : "<< progName << "[options] -i <input.ogv> -o <output.ogv> \n"
                 << " -- package and version \"" << PACKAGE_STRING << "\"\n\n"
                 << "Options are:\n"
                 << " -s time     : Start time in milliseconds from start\n"
                 << "               if no start time is given, 0 is assumed\n"
                 << " -e time     : End time in milliseconds\n"
                 << "               if no end time or -1 is given, the end of the\n"
                 << "               file is assumed\n"
                 << " -l length   : Length of the cut area\n"
                 << " -i input    : Input file (alternative) \n"
                 << " -o output   : Output file (alternative) \n";
}

void bufferedOutput(StreamMux& streamMux, double time, OggPacket packet)
{
  ListElement elem(time, packet);

  std::list<ListElement>::iterator iter(packetList.begin());
  while ((iter != packetList.end()) && (elem.time < iter->time))
    ++iter;
  packetList.insert(iter, elem);

  double lastTime(packetList.front().time);
  while ((lastTime - packetList.back().time) > bufferTime) {
    streamMux << packetList.back().packet;
    packetList.pop_back();
  }
}

void flushBuffer(StreamMux& streamMux)
{
  while (!packetList.empty()) {
    streamMux << packetList.back().packet;
    packetList.pop_back();
  }
}

/* TODO: take offset into account
 * The offset should be taken into account, so that the audio stream is
 * exactly as long as the video.
 *
 */

int oggCutCmd(int argc, char* argv[])
{
  int32 startTime(0);
  int32 endTime(-1);
  int32 length(-1);

  std::string inputFile;
  std::string outputFile;
  std::string programName(argv[0]);

  srand(time(0));

  int opt;
  while ((opt = getopt(argc, argv, "hi:o:s:e:l:")) != EOF)

    switch (opt) {

    case 'h':
      printHelpScreen(programName);
      exit(-1);

    case 'i':
      inputFile = std::string(optarg);
      break;

    case 'o':
      outputFile = std::string(optarg);
      break;

    case 's':
      startTime = _atoi(optarg);
      break;

    case 'e':
      endTime = _atoi(optarg);
      break;

    case 'l':
      length = _atoi(optarg); // yes, I know the atoi bug
      break;

    }

  argc -= optind;
  argv += optind;

  if ((argc > 2)) {
    printHelpScreen(programName);
    exit (-1);
  }

  if (argc > 0) {
    inputFile = std::string(argv[0]);
  }

  if (argc > 1) {
    outputFile = std::string(argv[1]);
  }

  /* Handle wrong parameter and parameter combinations */
  if (inputFile.empty() || outputFile.empty()) {
    printHelpScreen(programName);
    exit (-1);
  }

  if (startTime < 0) {
    logger.error() << "Error: start time is invalid\n";
    exit (-1);
  }

  if ((endTime > 0) && (length > 0)) {
    logger.warning() << "Warning: end time and length set, ignoring length\n";
  }

  if (endTime == -1) {
    if (length > 0) {
      endTime = startTime + length;
    } else {
      if (startTime == 0) {
        logger.error() << "No need to cut, just use copy!\n";
        exit (-1);
      }
    }
  }

  double startTimeSec(startTime*1.0/1000.0);
  double endTimeSec(endTime*1.0/1000.0);

  /* create a stream serializer */
  StreamSerializer streamSerializer;
  bool foundTheora(false);

  /* try to open the file. If there is no such file, abort with a message */
  if (!streamSerializer.open(inputFile)) {
    logger.error() << "Error: can not open file <"<<inputFile<<">\n";
    exit (-1);
  }

  /* create a repository for the new files and give the repository to the stream Muxer */
  FileRepository* outfile = new FileRepository(outputFile, MediaUnit::write);
  StreamMux streamMux(outfile);

  /* grep the configuration for all streams */
  std::vector<StreamConfig> streamConfigList;
  streamSerializer.getStreamConfig(streamConfigList);

  /* create the time synchronizer, which holds the time offset for every stream */
  std::vector<double> offset(streamConfigList.size(),0.0);

  /* Output some stream information and reset the offset */
  for (uint32 i(0); i<streamConfigList.size(); ++i) {
    StreamConfig& conf(streamConfigList[i]);
    logger.info() << "Stream No: "<<(int)conf.streamNo<<"(0x"<<std::hex
                  << conf.serialNo<<std::dec<<")\n";
    if (streamConfigList[i].parameter)
      // logger.info() << streamConfigList[i].parameter->toString();
      if (streamConfigList[i].type == OggType::theora)
        foundTheora = true;
    offset[i] = -1;
  }

  /* configure the output streams */
  streamMux.configureStreams(streamConfigList);

  /* */
  OggPacket packet;
  double    time;
  double    beginTime(0);
  bool      startMarker(false);

  /* copy the stream if the packets are within the cut area
   * and the first video keyframe has been found */
  while (streamSerializer.available()) {

    /* get the actual packet and it's time information */
    /* the time is meant to be the presentation start time */
    time = streamSerializer.getNextPacket(packet);

#ifdef DEBUG
    if (packet.getStreamType() == OggType::theora) {
      logger.debug() << "theora ";
    }

    if (packet.getStreamType() == OggType::vorbis) {
      logger.debug() << "vorbis ";
    }

    logger.debug() << time << std::endl;
#endif

    /* look deeper into the packets, if the belong into the cutting
     * area */
    if ((time >= startTimeSec) && (time < endTimeSec)) {

      /* are we within our cut interval and found the first keyframe? */
      if (!startMarker) {

        /* we are doing packet analysation by ourselfs - may be changed */
        if ((!foundTheora) || ((packet->getStreamType() == OggType::theora)
                               &&(!(packet->data()[0] & 0x40)))) {
          startMarker = true;
          beginTime   = time;
          offset[packet->getStreamNo()] = time;
        }
      }

      /* have we found the real starting position? */
      if (startMarker) {

        /* if this stream has no offset calculated, do it now */
        if (offset[packet->getStreamNo()] < 0) {
          offset[packet->getStreamNo()] = time;
#ifdef DEBUG
          logger.debug() << "offset for stream No <"<<(int)packet.getStreamNo()
                         <<"> is "<<offset[packet->getStreamNo()] - beginTime <<std::endl;
#endif // DEBUG
        }

        /* we need to bufferd the output to the stream, as the streams are not 100% in sync */
        bufferedOutput(streamMux, (time - offset[packet->getStreamNo()]), packet);
      }

    }

    /* the end of the cut area has reached */
    if (time >= endTimeSec) {
      break;
    }
  }

  /* first flush all buffers to be ordered correct */
  flushBuffer(streamMux);

  /* set the end of the stream and close the file,
   * which flushed the all internal stream encoder to flush all pages  */
  streamMux.setEndOfStream();
  streamMux.close();

  /* close the stream serializer with a big thank you */
  streamSerializer.close();

  return(0);
}

int main(int argc, char* argv[])
{
  try {
    return oggCutCmd(argc, argv);
  } catch (OggException & e) {
    logger.error() << "Fatal error: " << e.what() << std::endl;
    return -1;
  }
}

