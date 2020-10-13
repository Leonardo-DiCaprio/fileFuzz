/*
 * oggJoin will multiplex a number of video and audiostreams to one ogg file
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
#include <string>
#include <cstdlib>

#include "fileRepository.h"
#include "oggDecoder.h"
#include "oggEncoder.h"
#include "oggStreamDecoder.h"
#include "vorbisPosInterpreter.h"
#include "theoraPosInterpreter.h"
#include "oggPage.h"
#include "oggBOSExtractorFactory.h"
#include "exception.h"
#include "log.h"

struct JoinElement {
  FileRepository         repository;
  OggDecoder             decoder;
  GranulePosInterpreter* position;
  OggPage                nextPage;
  bool                   closed;
};

bool getNextPage(JoinElement* element, OggPage& page)
{
  if (element->decoder.isAvailable()) {
    /* if there is a packet available, get it */
    element->decoder >> page;
    return(true);
  }

  /* there is actually no packet available, so grap a data
   * bunch and transfer it into the decoder and see if we can
   * then extract a packet */

  while (element->decoder.isEmpty()) {

    /* if we can not grap any more data, return false */
    if (element->repository.isEndOfFile()) {
      element->closed = true;
      return(false);
    }

    /* transfer a new raw packet */
    RawMediaPacket rawPacket;
    element->repository >> rawPacket;
    element->decoder << rawPacket;

  }

  element->decoder >> page;
  return(true);
}

void printHelpScreen(const std::string& progName)
{
  logger.error() << "usage: " << progName
                 << " <outputFile> <inputFile1> <inputFile2> [ <inputFile3> [ ... ] ]\n"
                 << " -- package and version \"" << PACKAGE_STRING << "\"\n\n"
                 << "oggJoin will multiplex a number of ogg streams into \n"
                 << "one file. Actually every stream must be placed in a \n"
                 << "single file.\n\n";
}

int oggJoinCmd(int argc, char* argv[])
{

  std::string help("-h");
  if ((argc > 1) && (help == argv[1])) {
    printHelpScreen(argv[0]);
    exit(-1);
  }


  if (argc < 3) {
    printHelpScreen(argv[0]);
    exit(-1);
  }

  std::vector<JoinElement*> decoderList;

  /* open the m_repository and encoder for the joined file */
  FileRepository outRepository = FileRepository(argv[1], MediaUnit::write);
  OggEncoder oggEncoder;

  /* run through the file list given by the command line */
  for (uint32 i(2); i < (uint32)argc; ++i) {

    /* create a new element for one stream */
    JoinElement* newElement = new JoinElement;
    newElement->closed = false;
    newElement->repository = FileRepository(argv[i], MediaUnit::read);

    /* if we can not open the file, do not insert it in the decoder list */
    if (newElement->repository.isEndOfFile()) {
      logger.warning() << "Warning: can not open file <"<<argv[i]<<"> for reading\n\n";
      delete newElement;
    } else {

      /* get the first packet bunch from the file and place it into the decoder */
      RawMediaPacket packet;
      newElement->repository >> packet;
      newElement->decoder << packet;

      /* there must be at least the bos page */
      OggPage page;
      newElement->decoder >> page;

      ExtractorInformation config;
      if (!OggBOSExtractorFactory::extractInformation(page,config)) {
        logger.warning() << "Warning: <"<<argv[i]<<"> is not a valid ogg file";
        newElement->repository.close();
        delete newElement;
        continue;
      }

      newElement->position = OggBOSExtractorFactory::extractPositionInterpreter(config);

      /* if we found a valid stream, create the rest of the infrastructure */
      if (newElement->position != 0) {

        /* insert the BOS page into the new file (the first pages must be the BOS
         * pages) */
        oggEncoder << page;

        /* request the next page */
        getNextPage(newElement, newElement->nextPage);
        decoderList.push_back(newElement);
      } else {
        logger.warning() << "Warning: can not interpret ogg stream\n";
        /* we can not interpret the granual position of this stream,
         * so we close it */
        newElement->repository.close();
        delete newElement;
      }
    }
  }

  if (decoderList.empty()) {
    logger.error() << "Error: could not open any stream - abort\n";
    exit(-1);
  }

  uint32 closeCounter(0);

  /* run through the different streams and assemble them until there are no more pages */
  while (closeCounter < decoderList.size()) {

    double smallestTime(-10);
    uint32 smallestID(0);

    /* find the element, that should be inserted into the new file */
    for (uint32 i(0); i<decoderList.size(); ++i) {

      /* are there no more pages available? */
      if (decoderList[i]->closed)
        continue;

      double testTime(decoderList[i]->position->getTime(decoderList[i]->nextPage->granulepos()));
      if ((smallestTime < -9) || (smallestTime > testTime)) {
        smallestTime = testTime;
        smallestID   = i;
      }
    }

    /* insert the next page into the new file */
    oggEncoder << decoderList[smallestID]->nextPage;

    /* try to get the next page */
    if (!getNextPage(decoderList[smallestID], decoderList[smallestID]->nextPage)) {

      /* if this was the last page in this stream, clean up */
      decoderList[smallestID]->closed = true;
      decoderList[smallestID]->repository.close();
      delete decoderList[smallestID]->position;

      closeCounter++;
    }

    while (oggEncoder.isAvailable()) {
      RawMediaPacket outPacket;
      oggEncoder >> outPacket;
      outRepository << outPacket;
    }
  }

  /* cleanup the heap */
  for (uint32 i(0); i<decoderList.size(); ++i)
    delete decoderList[i];

  /* close the new file */
  outRepository.close();

  return(0);
}

int main(int argc, char* argv[])
{
  try {
    return oggJoinCmd(argc, argv);
  } catch (OggException & e) {
    logger.error() << "Fatal error: " << e.what() << std::endl;
    return -1;
  }
}

