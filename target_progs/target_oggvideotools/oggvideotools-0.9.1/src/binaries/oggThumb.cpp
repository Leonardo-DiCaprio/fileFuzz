/*
 * oggThumb creates thumbs from an ogg/theora video
 *
 * Copyright (C) 2008-2009 Joern Seger
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
#else
#include <getopt.h>
#endif

#include <vector>
#include <deque>
#include <limits>
#include <sstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <queue>

#include "fileRepository.h"
#include "streamSerializer.h"
#include "theoraDecoder.h"
#include "theoraStreamParameter.h"
#include "oggComment.h"
#include "rgbPlane.h"
#include "pictureLoader.h"
#include "pictureResize.h"
#include "exception.h"
#include "log.h"

const std::string validChars("0123456789,.x");

void extractUint32(std::deque<uint32>& list, const std::string& _argument,
                   char seperator)
{
  std::string argument(_argument);
  std::stringstream str;
  std::string substr;

  // if there is no argument given, the first frame will be created as a thumbnail
  if (argument.empty()) {
    list.push_back(0);
    return;
  }

  // delete all invalid data
  std::size_t pos;
  while ((pos = argument.find_first_not_of(validChars)) != std::string::npos) {
    logger.debug() << "erasing <"<<argument.at(pos)<<">\n";
    argument.erase(pos,1);
  }

  str << argument;

  uint32 value(0);
  while (!str.eof()) {
    std::stringstream part;
    getline(str, substr, seperator);
    part << substr;
    part >> value;
    list.push_back(value);
  }

}

void extractUint32Sort(std::deque<uint32>& list, const std::string& _argument,
                       char seperator)
{
  std::string argument(_argument);
  std::stringstream str;
  std::string substr;

  std::priority_queue<uint32> _list;

  // if there is no argument given, the first frame will be created as a thumbnail
  if (argument.empty()) {
    list.push_back(0);
    return;
  }

  // delete all invalid data
  std::size_t pos;
  while ((pos = argument.find_first_not_of(validChars)) != std::string::npos) {
    logger.debug() << "erasing <"<<argument.at(pos)<<">\n";
    argument.erase(pos,1);
  }

  str << argument;

  uint32 value(0);
  while (!str.eof()) {
    std::stringstream part;
    getline(str, substr, seperator);
    part << substr;
    part >> value;
    _list.push(value);
  }

  while (!_list.empty()) {
    list.push_front(_list.top());
    _list.pop();
  }
}


void extractDoubleSort(std::deque<double>& list, const std::string& _argument,
                       char seperator)
{
  std::string argument(_argument);
  std::stringstream str;
  std::string substr;

  std::priority_queue<double> _list;

  // if there is no argument given, the first frame will be created as a thumbnail
  if (argument.empty()) {
    list.push_back(0);
    return;
  }

  std::size_t pos;
  while ((pos = argument.find_first_not_of(validChars)) != std::string::npos)
    argument.erase(pos);

  str << argument;

  double value(0);
  while (!str.eof()) {
    std::stringstream part;
    getline(str, substr, seperator);
    part << substr;
    part >> value;
    _list.push(value);
  }

  while (!_list.empty()) {
    list.push_front(_list.top());
    _list.pop();
  }

}

void writeActualFrame(TheoraDecoder& decoder, std::deque<OggPacket>& packetList,
                      const std::string& name, uint32 width, uint32 height)
{
  th_ycbcr_buffer picture;
  RGBPlane plane;

  if (!TheoraDecoder::isPacketKeyframe(packetList[0])) {
    logger.error() << "first packet is not a keyframe\n";
    return; // could not happen ;-)
  }

  for (uint32 i(0); i<packetList.size(); ++i) {
    decoder << packetList[i];
    decoder >> picture;
  }

  plane = PictureLoader::importYCrCb_theora(picture, decoder.getWidth(), decoder.getHeight(), decoder.getInfo().pic_x, decoder.getInfo().pic_y, decoder.getInfo().pixel_fmt);

  PictureLoader::save(plane, name, width, height);

}

std::string getThumbname(const std::string& filename, const std::string& forcedThumbname,
                         const std::string& extension, uint32& counter, uint32 reqCount)
{
  std::stringstream thumbname;
  if (forcedThumbname.empty()) {
    std::size_t filenamestart = filename.find_last_of('/');
    std::size_t filenamelength = filename.find_last_of('.');

    if (filenamestart == std::string::npos)
      filenamestart = 0;
    else
      filenamestart++;

    if ((filenamelength != std::string::npos) && (filenamelength > filenamestart))
      filenamelength = filenamelength - filenamestart;
    else
      filenamelength = std::string::npos;

    thumbname << filename.substr(filenamestart,filenamelength);
    thumbname << "_" << counter++ << extension;
  } else if (reqCount == 1) {
    thumbname << forcedThumbname;
    thumbname << extension;
  } else {
    std::size_t replacePos = forcedThumbname.find_first_of('%');
    if (replacePos != std::string::npos ) {
      thumbname << forcedThumbname.substr(0, replacePos);
      thumbname << (counter++);
      thumbname << forcedThumbname.substr(replacePos + 1);
    } else {
      thumbname << forcedThumbname;
    }
    thumbname << extension;
  }
  return(thumbname.str());
}

void printHelpScreen(std::string& prog)
{
  logger.error() << "\nusage: "<<prog<<" [options] file1.ogv [ file2.ogv [ file3.ogv [...] ] ]\n"
                 << "Options:\n"
                 << " -t <time1, time2, time3, ...>         : create thumbnail from frame at time position time1, time2, time3 second\n"
                 << " -f <frameNo1, frameNo2, frameNo3, ...>: create thumbnail from frame number frameNo1, frameNo2, frameNo3\n"
                 << " -s <width>x<height>                   : resize to given values (if one argument is set to 0, it is calculated to meet the aspect ratio)\n"
                 << " -o <output format>                    : formats are jpg or png\n"
                 << " -n <filename>                         : force output filename\n"
                 << "The filename could be given with a %, which will be replaced by the actual picture number\n"
                 << "\n\n";

}

int oggThumbCmd(int argc, char* argv[])
{
  std::deque<double> timePosList;
  std::deque<uint32> frameNoList;

  uint32 width(0);
  uint32 height(0);
  uint32 requestedFrameCount(0);

  std::string programName(argv[0]);
  std::string extension(".jpg");
  std::string forcedThumbname;

  int opt;

  enum {
    opt_help = 256,
    opt_verbose
  };

//#ifdef with_eclipse_CDTBUG
  option longOpts[] = {
    { /* name: */      "help"
      ,     /* has_arg: */   0,
      /* flag: */      NULL,
      /* val: */       opt_help
    },
    { /* name:    */    "verbose",
      /* has_arg: */    1,
      /* flag:    */    NULL,
      /* val:     */    opt_verbose
    }
  };
//#endif

  while ((opt = getopt_long(argc, argv, "hf:t:s:o:n:v:", longOpts, NULL)) != EOF)
    switch (opt) {

    case '?':
    case 'h':
    case opt_help:
      printHelpScreen(programName);
      return -1;

    case 'f':
      extractUint32Sort(frameNoList, optarg, ',');
      break;

    case 't':
      extractDoubleSort(timePosList,optarg, ',');
      break;

    case 's': {
      std::deque<uint32> framesize;
      extractUint32(framesize, optarg, 'x');
      if (framesize.size() != 2) {
        logger.error() << "please specify the size in the following way: -s320x480\n";
        return -1;
      }
      width = framesize[0];
      height = framesize[1];
    }
    break;

    case 'o':
      extension = optarg;
      extension = "." + extension;
      break;

    case 'n':
      forcedThumbname = optarg;
      std::size_t extendPos;
      if ((extendPos = forcedThumbname.find_last_of(".")) != std::string::npos) {
        extension = forcedThumbname.substr(extendPos);
        forcedThumbname = forcedThumbname.substr(0,extendPos);
      }
      logger.debug() << "Forced thumbnail name is "<<forcedThumbname<<std::endl;
      break;
    case 'v':
    case opt_verbose: {
      std::string vstr(optarg);
      std::stringstream tempStream;
      int verbosity(0);

      if (vstr == "debug") {
        logger.setLevel(OggLog::LOG_DEBUG);
      } else if (vstr == "info") {
        logger.setLevel(OggLog::LOG_INFO);
      } else if (vstr == "warning") {
        logger.setLevel(OggLog::LOG_WARNING);
      } else if (vstr == "error") {
        logger.setLevel(OggLog::LOG_ERROR);
      } else {
        tempStream << vstr;
        tempStream >> verbosity;
        if (tempStream.fail() || verbosity < 0 || verbosity > 3) {
          logger.error() << "Error: Invalid verbosity \"" << optarg << "\"\n";
          return(-1);
        }
        switch (verbosity) {
        case 0:
          logger.setLevel(OggLog::LOG_ERROR);
          break;
        case 1:
          logger.setLevel(OggLog::LOG_WARNING);
          break;
        case 2:
          logger.setLevel(OggLog::LOG_INFO);
          break;
        case 3:
          logger.setLevel(OggLog::LOG_DEBUG);
        }
      }
      break;
    }
    }

  argc -= optind;
  argv += optind;

  requestedFrameCount = frameNoList.size() + timePosList.size();

  if (argc == 0) {
    logger.error() << "Please specify at least one ogg file\n";
    return -1;
  }

  logger.info() << "Creating thumbs under the following option:\n";

  if (!timePosList.empty()) {
    logger.info() << "Frames at time (in seconds): ";
    for (uint32 i(0); i<timePosList.size(); ++i)
      logger.info() << timePosList[i] <<"  ";
    logger.info() << "\n";
  }

  if (!frameNoList.empty()) {
    logger.info() << "Frame numbers: ";
    for (uint32 i(0); i<frameNoList.size(); ++i)
      logger.info() << frameNoList[i] <<"  ";
    logger.info() << "\n";
  }

  if (width)
    logger.info() << "width is set to: "<<width<<"\n";

  if (height)
    logger.info() << "height is set to: "<<height<<"\n";

  logger.info() << "file type: " << extension << "\n";

  logger.info() << "The following ogg media files will be used: ";
  for (int i(0); i<argc; ++i)
    logger.info() << argv[i] << " ";

  logger.info() << "\n";
  uint32 counter(0);

  // go through the files
  for (int i(0); i<argc; ++i) {

    /* create the stream serializer */
    StreamSerializer streamSerializer;
    TheoraDecoder theoraDecoder;
    uint8 foundTheora(0);
    std::string filename(argv[i]);
    double aspectCorrection;

    std::deque<double> tmptimePosList = timePosList;
    std::deque<uint32> tmpframeNoList = frameNoList;

    if (forcedThumbname.empty())
      counter=0;

    if (!streamSerializer.open(filename)) {
      logger.error() << "Error: can not open file <"<<filename<<">\n";
      continue;
    }

    uint8 theoraStreamNo(0);

    /* create the headers */
    std::vector<StreamConfig> streamConfigList;
    streamSerializer.getStreamConfig(streamConfigList);

    std::vector<OggComment> oggComments;

    /* Output some stream information */
    for (uint32 i(0); i<streamConfigList.size(); ++i) {

      if (streamConfigList[i].type == OggType::theora) {
        // take the first theora stream
        if (!foundTheora) {
          theoraStreamNo = streamConfigList[i].streamNo;
          TheoraStreamParameter& theoraConfig = dynamic_cast<TheoraStreamParameter&>(*streamConfigList[i].parameter.get());
          theoraDecoder.initDecoder(streamConfigList[i], oggComments);
          logger.info() << "Info:\n" << theoraDecoder.configuration()<<std::endl;

          if (theoraDecoder.getInfo().aspect_numerator > 0 && theoraDecoder.getInfo().aspect_denominator > 0)
            aspectCorrection = (theoraDecoder.getInfo().aspect_numerator*1.0)/(theoraDecoder.getInfo().aspect_denominator*1.0);
          else
            aspectCorrection = 1;

          if ((width == 0) && (height == 0)) {
            width = theoraConfig.pictureX * aspectCorrection; //theoraConfig.frameX;
            height = theoraConfig.pictureY; //theoraConfig.frameY;
          } else {
            if (height == 0)
              height = (uint32)((width * theoraConfig.pictureY*1.0)/(theoraConfig.pictureX*aspectCorrection*1.0) + 0.5);
            else if (width == 0)
              width = (uint32)((height * theoraConfig.pictureX*aspectCorrection*1.0)/(theoraConfig.pictureY*1.0) +0.5);
          }

          logger.info() << "width: "<<width<<" and height: "<<height<<"\n";

        }
        foundTheora++;
      }
    }

    if (!foundTheora) {
      logger.warning() << "There is no theora stream in file <"<<filename<<">\n";
      continue;
    }

    if (foundTheora > 2)
      logger.warning() << "Found more than one theora stream in file <"<<filename<<"> using first stream\n";

    /* set up first time/frame */
    double nextTime;
    uint32 nextFrame;

    bool noMoreTime(false);
    bool noMoreFrame(false);

    if (tmpframeNoList.empty()) {
      nextFrame = std::numeric_limits<uint32>::max();
      noMoreFrame = true;
    } else {
      nextFrame = tmpframeNoList.front();
      tmpframeNoList.pop_front();
    }

    if (tmptimePosList.empty()) {
      nextTime = std::numeric_limits<double>::max();
      noMoreTime = true;
    } else {
      nextTime = tmptimePosList.front();
      tmptimePosList.pop_front();
    }

    std::deque<OggPacket> packetList;
    double    time;
    OggPacket packet;

    while (streamSerializer.available()) {

      // get the next packet
      time = streamSerializer.getNextPacket(packet);

      // is this packet a theora frame
      if (packet->getStreamType() != OggType::theora)
        continue;

      // write actual time
      logger.info() << "\r "<<time<<"     ";

      // if this is a keyframe, we are able to decode from this
      if (TheoraDecoder::isPacketKeyframe(packet)) {
        packetList.clear();
      }

      // store packets for decoding since last keyframe
      packetList.push_back(packet);

      // should this packet be written due to frame number comparison?
      if (nextFrame == packet->getPacketNo()) {

        if (tmpframeNoList.empty()) {
          nextFrame = std::numeric_limits<uint32>::max();
          noMoreFrame = true;
        } else {
          nextFrame = tmpframeNoList.front();
          tmpframeNoList.pop_front();
        }


        std::string thumbname = getThumbname(filename, forcedThumbname, extension,
                                             counter, requestedFrameCount);
        logger.info() << "writing "<<thumbname<<std::endl;
        writeActualFrame(theoraDecoder, packetList, thumbname, width, height);
      }

      // should this packet be written due to time limit?
      if (time >= nextTime) {

        if (tmptimePosList.empty()) {
          nextTime = std::numeric_limits<uint32>::max();
          noMoreTime = true;
        } else {
          nextTime = tmptimePosList.front();
          tmptimePosList.pop_front();
        }

        std::string thumbname = getThumbname(filename, forcedThumbname, extension,
                                             counter, requestedFrameCount);

        logger.info() << "writing "<<thumbname<<std::endl;
        writeActualFrame(theoraDecoder, packetList, thumbname, width, height);

      }

      if (noMoreTime && noMoreFrame)
        break;
    }

    streamSerializer.close();

  }


  logger.info() << std::endl;

#ifdef OSX_MALLOC_DEBUG
  std::cout << "Done!\n";
  while (1==1) { }
#endif

  return(0);
}

int main(int argc, char* argv[])
{
  try {
    return oggThumbCmd(argc, argv);
  } catch (OggException & e) {
    logger.error() << "Fatal error: " << e.what() << std::endl;
    return -1;
  }
}

