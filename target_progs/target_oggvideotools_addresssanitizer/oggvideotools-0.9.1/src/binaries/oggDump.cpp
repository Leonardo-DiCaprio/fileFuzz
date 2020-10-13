/*
 * oggDump will dump out an ogg file either by packets or by pages
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
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <ostream>
#include <cstdlib>
#include <unistd.h>

#include "fileRepository.h"
#include "rawMediaPacket.h"
#include "oggDecoder.h"
#include "oggEncoder.h"
#include "oggStreamDecoder.h"
#include "oggPacket.h"
#include "oggBOSExtractorFactory.h"
#include "streamSerializer.h"
#include "exception.h"
#include "log.h"

struct OutputUnit {
  OggEncoder     encoder;
  FileRepository repository;
};

void printHelp(std::string programName)
{
  logger.error() << "usage <"<<programName<<"> [options] file" << std::endl;
  logger.error() << "Options are:\n"
                 << " -h         : help screen        \n"
                 << " -g         : dump pages         \n"
                 << " -p         : dump packets       \n"
                 << " -l <level> : information depth; default: 5 (most information)\n"
                 << " -s         : promt for streams to dump\n"
                 << " -o <file>  : output dump information to a file\n";
}

void dumpPacketof(std::string& file, uint8 dumpLevel, bool promptForStreams, std::string& outFilename)
{
  /* open the first file to be read */
  std::vector<StreamConfig> configList;
  StreamSerializer serializer;

  std::ofstream outStream;

  /* if there is a filename given, write the data to this file */
  if (!outFilename.empty())
    outStream.open(outFilename.c_str());

  if (!serializer.open(file)) {
    logger.error() << "Can not open file <" << file << ">\n";
    exit(-1);
  }

  /* read the stream configuration */
  serializer.getStreamConfig(configList);

  std::vector<uint32> outputStreamNo;

  for (uint32 i(0); i<configList.size(); ++i) {
    std::cout << "Config of stream No. "<<i<<"\n"
              << "StreamType: ";
    switch (configList[i].type) {
    case OggType::kate:
      std::cout << "kate\n";
      break;
    case OggType::theora:
      std::cout << "theora\n";
      break;
    case OggType::vorbis:
      std::cout << "vorbis\n";
      break;
    }

    std::cout << "serial No : 0x"<< std::hex << configList[i].serialNo;
    if (configList[i].parameter)
      //std::cout << configList[i].parameter->toString();

      if (promptForStreams) {
        std::cout << "Dump this stream? (y/n) \n";
        char answer;
        std::cin >> answer;

        if (answer != 'Y' && answer != 'y')
          outputStreamNo.push_back(configList[i].streamNo);

        std::cout << answer << "\n";
      }
  }

  OggPacket packet;
  double _time;
  bool print;

  while (serializer.available()) {

    _time = serializer.getNextPacket(packet);

    print = true;
    for (uint32 i(0); i<outputStreamNo.size(); ++i)
      if (outputStreamNo[i] == packet->getStreamNo())
        print = false;

    if (!print)
      continue;

    if (outFilename.empty()) {
      std::cout << "\nTime: " << _time;
      std::cout << packet->toString(dumpLevel);
    } else {
      outStream << "\nTime: " << _time;
      outStream << packet->toString(dumpLevel);
    }
  }
}

int oggDumpCmd(int argc, char* argv[])
{

  /* default values
   * for the command line arguments */

  uint8 dumpLevel(5);
  std::string outFilename("");
  bool dumpPages(false);
  bool dumpPackets(false);
  bool promptForStreams(false);

  std::string programName(argv[0]);

  int opt;
  while ((opt = getopt(argc, argv, "hgpl:so:")) != EOF)

    switch (opt) {

    case 'h':
      printHelp(argv[0]);
      exit(-1);

    case 'g':
      dumpPages = true;
      break;

    case 'p':
      dumpPackets = true;
      break;

    case 's':
      promptForStreams = true;
      break;

    case 'o':
      outFilename = std::string(optarg);
      break;

    case 'l':
      dumpLevel = atoi(optarg); // yes, I know the atoi bug
      break;

    }

  argc -= optind;
  argv += optind;

  std::string analysisFile;

  if (argc == 1)
    analysisFile = std::string(argv[0]);
  else {
    printHelp(programName);
    exit(-1);
  }

  if ((!dumpPages) && (!dumpPackets)) {
    logger.error() << "Specify whether you want to dump pages, packet or both by -g and/or -p\n";
    exit(-1);
  }

  if (dumpPackets) {
    dumpPacketof(analysisFile, dumpLevel, promptForStreams, outFilename);
    return(0);
  }

  std::ofstream outStream;

  /* if there is a filename given, write the data to this file */
  if (!outFilename.empty())
    outStream.open(outFilename.c_str());

  /* open the m_repository
   in this easy example, it is a simple file */
  FileRepository repository(analysisFile, MediaUnit::read);

  OggDecoder oggDecoder;
  std::map<uint32, OggStreamDecoder> oggStreamDecoderList;
  std::vector<OggPage> bosPages;

  /* run through the m_repository until there is no data left */
  while (!repository.isEndOfFile()) {

    RawMediaPacket rawDecoderPacket;

    /* extract a raw data bunch from the file .. */
    repository >> rawDecoderPacket;

    /* .. and insert it into the ogg decoder */
    oggDecoder << rawDecoderPacket;

    /* are there any complete ogg Pages available ? */
    while (oggDecoder.isAvailable()) {

      OggPage oggPage;

      /* grap the next page */
      oggDecoder >> oggPage;

      /* what ID has this page / what stream does this page belongs to */
      uint32 serialID = oggPage->serialno();

      if (oggPage->isBOS()) {

        bool addPage(false);

        switch (OggBOSExtractorFactory::getStreamType(oggPage)) {

        case OggType::theora: {
          std::cout << "Found theora stream with ID= 0x" << std::hex
                    << serialID << std::dec << std::endl;
          if (promptForStreams) {
            std::cout << "Dump this stream? (y/n) \n";
            char answer;
            std::cin >> answer;
            if (answer == 'Y' || answer == 'y')
              addPage = true;
            std::cout << answer << "\n";
          } else
            addPage = true;
        }
        break;

        case OggType::vorbis: {
          std::cout << "Found vorbis stream with ID= 0x" << std::hex
                    << serialID << std::dec << std::endl;
          if (promptForStreams) {
            std::cout << "Dump this stream? (y/n) ";
            char answer;
            std::cin >> answer;
            if (answer == 'Y' || answer == 'y')
              addPage = true;
            std::cout << answer << "\n";
          } else
            addPage = true;

        }
        break;

        case OggType::kate: {
          std::cout << "Found kate stream with ID= 0x" << std::hex
                    << serialID << std::dec << std::endl;
          if (promptForStreams) {
            std::cout << "Dump this stream? (y/n) ";
            char answer;
            std::cin >> answer;
            if (answer == 'Y' || answer == 'y')
              addPage = true;
            std::cout << answer << "\n";
          } else
            addPage = true;

        }
        break;

        default: {
          std::cout << "Found unknown stream with ID= 0x" << std::hex
                    << serialID << std::dec << std::endl;
          if (promptForStreams) {
            std::cout << "Dump this stream? (y/n) \n";
            char answer;
            std::cin >> answer;
            if (answer == 'Y' || answer == 'y')
              addPage = true;
            std::cout << answer << "\n";
          } else
            addPage = true;
        }
        break;
        }
        if (addPage) {
          oggStreamDecoderList[serialID] = OggStreamDecoder();
          oggStreamDecoderList[serialID] << oggPage;
          bosPages.push_back(oggPage);
        }

      } else {

        /* does the user want to dump this stream */
        if (oggStreamDecoderList.find(serialID) != oggStreamDecoderList.end()) {

          if (dumpPages) {

            std::string outputString;

            // are there any bos pages, then toString them first
            if (!bosPages.empty()) {
              for (uint32 j(0); j<bosPages.size(); ++j)
                outputString += bosPages[j]->toString(dumpLevel);
              bosPages.clear();
            }

            outputString += oggPage->toString(dumpLevel);

            if (outFilename.empty())
              std::cout << outputString;
            else
              outStream << outputString;

          }

        }
      }
    }
  }

  /* close all files */
  repository.close();
  if (!outFilename.empty())
    outStream.close();

  return (0);
}

int main(int argc, char* argv[])
{
  //logger.setLevel(OggLog::LOG_DEBUG);
  try {
    return oggDumpCmd(argc, argv);
  } catch (OggException & e) {
    logger.error() << "Fatal error: " << e.what() << std::endl;
    return -1;
  }
}

