/*
 * oggSplit will demultiplex a number of video and audio streams from an ogg file
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
#include <map>
#include <sstream>
#include <string>
#include <cstdlib>

#include "fileRepository.h"
#include "rawMediaPacket.h"
#include "oggDecoder.h"
#include "oggEncoder.h"
#include "oggStreamDecoder.h"
#include "oggBOSExtractorFactory.h"
#include "exception.h"
#include "log.h"

struct OutputUnit {
  OggEncoder encoder;
  FileRepository repository;
};
void printHelpScreen(const std::string& progName)
{
  logger.error() << "usage: " << progName << " <file.ogv>\n"
                 << " -- package and version \"" << PACKAGE_STRING << "\"\n\n"
                 << "oggSplit demultiplexes an ogg file into its streams.\n"
                 << "Every stream is placed into a single file, which are\n"
                 << "called theora_<serialNo>.ogg or vorbis_<serialNo>.ogg.\n"
                 << "The serial number is the unique ogg serial number of\n"
                 << "this stream.\n\n";
}

int oggSplitCmd(int argc, char* argv[])
{

  if (argc != 2) {
    printHelpScreen(argv[0]);
    exit(-1);
  }

  std::string help("-h");

  if (help == argv[1]) {
    printHelpScreen(argv[0]);
    exit(-1);
  }

  /* open the m_repository
   in this easy example, it is a simple file */
  FileRepository repository(argv[1], MediaUnit::read);

  /* open the file to write the new stream */
  std::map<uint32, OutputUnit> outputFileList;

  RawMediaPacket rawDecoderPacket;
  OggDecoder oggDecoder;

  /* run through the m_repository until there is no data left */
  while (!repository.isEndOfFile()) {

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

      /* if this is the start of a stream, create a m_repository file for it */
      if (oggPage->isBOS()) {

        std::stringstream filename;

        switch (OggBOSExtractorFactory::getStreamType(oggPage)) {

        case OggType::theora:
          filename << "theora_" << std::hex << serialID << std::dec
                   << ".ogv";
          break;

        case OggType::vorbis:
          filename << "vorbis_" << std::hex << serialID << std::dec
                   << ".oga";
          break;

        case OggType::kate:
          filename << "kate_" << std::hex << serialID << std::dec
                   << ".ogv";
          break;

        default:
          logger.warning() << "unknown type ID "<< std::hex << serialID << std::dec <<"\n";
          filename << "unknown_" << std::hex << serialID << std::dec
                   << ".ogv";

        }

        logger.info() << "creating file <"<<filename.str()<<">\n";
        outputFileList[serialID].repository = FileRepository(
                                                filename.str(), MediaUnit::write);
        outputFileList[serialID].encoder = OggEncoder();
      }

      /* if this is a simple page, insert it into the decoder it
       belongs to */
      outputFileList[serialID].encoder << oggPage;
      while (outputFileList[serialID].encoder.isAvailable()) {

        RawMediaPacket rawOutput;

        /* extract the raw packets */
        outputFileList[serialID].encoder >> rawOutput;
        outputFileList[serialID].repository << rawOutput;

      }
      if (oggPage->isEOS())
        outputFileList[serialID].repository.close();
    }
  }

  repository.close();

  return (0);
}

int main(int argc, char* argv[])
{
  try {
    return oggSplitCmd(argc, argv);
  } catch (OggException & e) {
    logger.error() << "Fatal error: " << e.what() << std::endl;
    return -1;
  }
}

