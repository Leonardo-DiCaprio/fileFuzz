/*
 * oggLength is a command line tool, to return the length of an ogg file
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
#include <string>
#include <cstdlib>
#include <unistd.h>

#include "fileRepository.h"
#include "streamSerializer.h"
#include "exception.h"
#include "log.h"

void printHelpScreen(std::string& progName)
{
  logger.error() << "usage: " << progName << " <inputFile> \n";
}

int oggLengthCmd(int argc, char* argv[])
{

  std::string inputFile;
  std::string programName(argv[0]);

  bool printVorbisExtra(false);
  bool printTheoraExtra(false);

  int opt;
  while ((opt = getopt(argc, argv, "hvtVT")) != EOF)

    switch (opt) {

    case 'h':
      printHelpScreen(programName);
      exit(-1);

    case 'v':
    case 'V':
      printVorbisExtra = true;
      break;

    case 't':
    case 'T':
      printTheoraExtra = true;
      break;

    }

  argc -= optind;
  argv += optind;

  if (argc == 1)
    inputFile = std::string(argv[0]);
  else {
    printHelpScreen(programName);
    exit(-1);
  }

  /* create the stream serializer */
  StreamSerializer streamSerializer;

  /* open the file */
  if (!streamSerializer.open(inputFile)) {
    logger.error() << "Error: can not open file <"<<inputFile<<">\n";
    exit(-1);
  }

  /* create the headers */
  std::vector<StreamConfig> streamConfigList;
  streamSerializer.getStreamConfig(streamConfigList);

//  for (uint32 i(0); i<streamConfigList.size(); ++i) {
//    logger.info() << streamConfigList[i].parameter->toString();
//  }

  OggPacket packet;
  double timeVorbis;
  double timeTheora;
  double time;
  double retTime(-1);

  /* copy the stream if the packets are within the cut area
   * and the first video keyframe has been found */
  while (streamSerializer.available()) {

    time = streamSerializer.getNextPacket(packet);

    if (time < 0) {
      break;
    } else if (packet->getStreamType() == OggType::theora)
      timeTheora = time;
    if (packet->getStreamType() == OggType::vorbis)
      timeVorbis = time;

    retTime = time;

  }

  std::cout << (int)(retTime*1000) << std::endl;

  if (printVorbisExtra)
    std::cout << "  Vorbis End-Time (packet basis): " << (int)(timeVorbis*1000) << std::endl;

  if (printTheoraExtra)
    std::cout << "  Theora End-Time               : " << (int)(timeTheora*1000) << std::endl;

  return((int)(retTime*1000));
}

int main(int argc, char* argv[])
{
  try {
    return oggLengthCmd(argc, argv);
  } catch (OggException & e) {
    logger.error() << "Fatal error: " << e.what() << std::endl;
    return -1;
  }
}

