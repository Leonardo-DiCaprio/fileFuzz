/*
 * oggScroll is a tool to scroll through a theora stream
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
#include <termios.h>
#include <SDL/SDL.h>

#include "fileRepository.h"
#include "streamSerializer.h"
#include "theoraDecoder.h"
#include "theoraStreamParameter.h"
#include "SDLvideoOutput.h"
#include "exception.h"
#include "log.h"

void printHelpScreen(std::string& progName)
{
  logger.error() << "usage: " << progName << "[options]\n";
}

int getChar()
{
  termios origTerminal;
  termios tempTerminal;
  int character;

  /* get the actuall terminal state */
  if ((tcgetattr (STDIN_FILENO, &tempTerminal)) == -1)
    exit(-1);

  origTerminal = tempTerminal;


  tempTerminal.c_iflag =
    tempTerminal.c_iflag & ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  tempTerminal.c_oflag = tempTerminal.c_iflag & ~(OPOST);
  tempTerminal.c_cflag = tempTerminal.c_cflag & ~(CSIZE | PARENB);
  tempTerminal.c_lflag = tempTerminal.c_lflag & ~(ECHO|ICANON|IEXTEN|ISIG);
  tempTerminal.c_cflag = tempTerminal.c_cflag | CS8;
  tempTerminal.c_cc[VMIN]  = 1;
  tempTerminal.c_cc[VTIME] = 0;

  /*Jetzt setzten wir den raw-Modus*/
  /*
   *    if ((tcsetattr (fd, TCSAFLUSH, &new_io)) == -1)
   */

  /* change terminal to raw behaviour */
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &tempTerminal);
//  cfmakeraw(&tempTerminal);

  /* wait for a keypress event */
  character = getchar();

  /* restore the old terminal */
  tcsetattr (STDIN_FILENO, TCSANOW, &origTerminal);

  return(character);
}

int oggScrollCmd(int argc, char* argv[])
{
  std::string inputFile;
  std::string programName(argv[0]);

  int opt;
  while ((opt = getopt(argc, argv, "h")) != EOF)

    switch (opt) {

    case 'h':
      printHelpScreen(programName);
      exit(-1);

    }

  argc -= optind;
  argv += optind;

  if (argc == 1)
    inputFile = std::string(argv[0]);
  else {
    printHelpScreen(programName);
    exit(-1);
  }

  if (inputFile.empty()) {
    logger.error() << "Error: please define an input and output file with -i and -o\n\n";
    return(-1);
  }

  /* create the */
  StreamSerializer streamSerializer;
  TheoraDecoder    theoraDecoder;
  uint8            foundTheora(0);
  std::vector<OggComment> commentList;

  if (!streamSerializer.open(inputFile)) {
    logger.error() << "Error: can not open file <"<<inputFile<<">\n";
    return(-1);
  }

  uint8 theoraStreamNo(0);

  /* create the headers */
  std::vector<StreamConfig> streamConfigList;
  streamSerializer.getStreamConfig(streamConfigList);

  TheoraStreamParameter* theoraConfig(0);

  /* Output some stream information */
  for (uint32 i(0); i<streamConfigList.size(); ++i) {
    if (streamConfigList[i].type != ogg_unknown) {
      logger.info() << streamConfigList[i].parameter->toString();
    }
    if (streamConfigList[i].type == ogg_theora) {
      // take the first theora stream
      if (!foundTheora) {
        theoraStreamNo = streamConfigList[i].streamNo;
        theoraConfig   = (TheoraStreamParameter*)streamConfigList[i].parameter;
        theoraDecoder.initDecoder(streamConfigList[i], commentList);
      }
      foundTheora++;
    }
  }

  OggPacket packet;
  double time;

  if (foundTheora == 0) {
    logger.error() << "I did not find any theora stream\n";
    exit(-1);
  }

  /* create SDL video object */
  VideoInfo sdlInfo;
  sdlInfo.frame_width  = theoraConfig->pictureX;
  sdlInfo.frame_height = theoraConfig->pictureY;
  sdlInfo.offset_x=0;
  sdlInfo.offset_y=0;

  //TheoraVideoPacket picture;
  th_ycbcr_buffer picture;
  SDLvideoOutput videoOut;
  videoOut.init(sdlInfo);

  /* play back the pictures */
  bool keyframeSearch(false);
  bool avail(true);

  while (1==1) {

    time = streamSerializer.getNextPacket(packet);
    while (avail) {
//      logger.debug() << time<<"   -   Stream No: "<<(int)packet.getStreamNo()<< "  theora Stream No: "<<(int)theoraStreamNo<<"\n";
      if ((packet.getStreamNo() != theoraStreamNo) || ((keyframeSearch) && (!TheoraDecoder::isPacketKeyframe(packet)))) {
        if (!streamSerializer.available())
          avail = false;
        else
          time = streamSerializer.getNextPacket(packet);
      } else
        break;

    }

    if (!avail)
      break;

    theoraDecoder << packet;
    theoraDecoder >> picture;
    videoOut      << picture;

    logger.info() << "\rTime: "<<time<<"                    ";

    int key(getChar());
    if (key == 'q')
      break;

    if (key == '+')
      keyframeSearch=true;
    else
      keyframeSearch=false;
  }


  return(0);
}

int main(int argc, char* argv[])
{
  try {
    return oggScrollCmd(argc, argv);
  } catch (OggException & e) {
    logger.error() << "Fatal error: " << e.what() << std::endl;
    return -1;
  }
}

