/*
 * oggResize creates a resized video
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
#endif

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>

#include <cstring>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/lexical_cast.hpp>

#include "definition.h"
#include "helper.h"

#include "rgbPlane.h"
#include "pictureBlend.h"
#include "pictureLoader.h"
#include "oggComment.h"

#include "videoHook.h"
#include "audioHook.h"

#include "fileRepository.h"
#include "streamSerializer.h"
#include "streamMux.h"

#include "cmdlineextractor.h"
#include "wishList.h"
#include "exception.h"
#include "log.h"

void printHelpScreen(const std::string& progname)
{
  logger.error() << "usage: "<<progname << " -- package and version \""
                 << PACKAGE_STRING << "\"\n\n"
                 << " [Options] originalFile.ogv newFile.ogv\n" << "  Option: \n"
                 << "    -h     this helpscreen\n"
                 << "    -o     specify output file name (optional)\n"
                 << "    -s     <width x height>  new frame size\n"
                 << "    -f     video framerate in frames per second\n"
                 << "    -F     audio samplerate in Hz\n"
                 << "    -d     video datarate in Bit/s\n"
                 << "    -D     audio datarate in Bit/s\n"
                 << "    -q     theora video quality\n"
                 << "    -N     audio channels\n"
                 << "    -a     add png with alpha channel on top of a frame\n"
                 << "           before the resize process \n"
                 << "           form: file.png[,<startTime>[,<endTime>[,s]]]\n"
                 << "           start and end time is in seconds and can be a floating point\n"
                 << "    -A     add png with alpha channel on top of a frame\n"
                 << "           after the resize process (same parameters as -a)\n"
                 << "    -p     only use every <x>th packet to create the new video\n"
                 << "    -c     comments for the video stream\n"
                 << "    -C     comments for the audio stream\n"
                 << "    -x     do not ask for overwriting output file\n"
                 << "    -rv    force reencode video stream\n"
                 << "    -ra    force reencode audio stream\n"
                 << "    -Q     resize quality (1=best/slow; 6=worst/fast)\n"
                 << "    -t     stretch picture to new size\n\n";

}

void readPictures(std::vector<BlendElement>& blendList)
{
  for (uint32 i(0); i < blendList.size(); ++i) {
    RGBPlane plane;
    try {
      PictureLoader::load ( plane, blendList[i].getPictureName() );
      blendList[i].setPicturePlane( plane );
    } catch (std::exception & e) {
      logger.error() << "Error: " << e.what() << std::endl;
    }

  }
}

/* you can create a alpha blend object with the following option
 * -a picturex.png,1.23,2.34;picturey.png,5.12,7,s */

void alphaBlend(double time, RGBPlane& inPlane,
                std::vector<BlendElement>& blendList, float intensityStair)
{

  for (uint32 i( 0); i<blendList.size(); ++i) {
    switch (blendList[i].state) {
    case BlendElement::blend_off: {
      if (time >= blendList[i].startTime) {
        if (blendList[i].smooth) {
          blendList[i].state = BlendElement::blend_slideIn;
        } else {
          blendList[i].intensity = 1.0;
          blendList[i].state = BlendElement::blend_on;
        }
      }
    }
    break;

    case BlendElement::blend_slideIn: {
      blendList[i].intensity += intensityStair;

      if (blendList[i].intensity >= 1.0) {
        blendList[i].state = BlendElement::blend_on;
        blendList[i].intensity = 1.0;
      }

    }
    break;

    case BlendElement::blend_on: {
      if ( (blendList[i].endTime > 0.0 )
           && (time >= blendList[i].endTime )) {
        if (blendList[i].smooth) {
          blendList[i].state = BlendElement::blend_slideOut;
        } else {
          blendList[i].intensity = 0.0;
          blendList[i].state = BlendElement::blend_end;
        }
      }
    }
    break;

    case BlendElement::blend_slideOut: {
      blendList[i].intensity -= intensityStair;

      if (blendList[i].intensity <= 0.0) {
        blendList[i].state = BlendElement::blend_end;
        blendList[i].intensity = 0.0;
      }

    }
    break;

    case BlendElement::blend_end: {
      /* do nothing */
    }
    break;

    }

    if ( (blendList[i].state != BlendElement::blend_end )
         && (blendList[i].state != BlendElement::blend_off ))
      inPlane = PictureBlend::alphaBlend(inPlane, blendList[i].picture,
                                         blendList[i].intensity);

  }

}

void analyseVideoTranscoding(WishList& wishList,
                             std::shared_ptr<TheoraStreamParameter> theoraConfigInput,
                             std::shared_ptr<TheoraStreamParameter> theoraConfigOutput)
{
  /* first we believe the output should be equal for all
   * parameters that are not explicitly changed */
  *theoraConfigOutput.get() = *theoraConfigInput.get();

  if (wishList.changeVideoDatarate) {
    if (theoraConfigInput->videoBitrate != wishList.videoDatarate) {
      theoraConfigOutput->videoBitrate = wishList.videoDatarate;
      theoraConfigOutput->videoQuality = 0;
    }
  }

  if (wishList.changeVideoQuality) {
    if (theoraConfigInput->videoQuality != wishList.videoQuality) {
      theoraConfigOutput->videoBitrate = 0;
      theoraConfigOutput->videoQuality = wishList.videoQuality;
    }
  }

  if (wishList.changeSize) {
    if ( (theoraConfigInput->pictureX != wishList.width )
         || (theoraConfigInput->pictureY != wishList.height )
         || (theoraConfigInput->aspectRatioNum != 1 )
         || (theoraConfigInput->aspectRatioDenom != 1 )) {

      theoraConfigOutput->pictureX = wishList.width;
      theoraConfigOutput->pictureY = wishList.height;

      theoraConfigOutput->frameX  = (theoraConfigOutput->pictureX+15)&~0xF;
      theoraConfigOutput->frameY  = (theoraConfigOutput->pictureY+15)&~0xF;

      // We force the offset to be even.
      // This ensures that the chroma samples align properly with the luma
      // samples.

      theoraConfigOutput->frameXOffset  = ((theoraConfigOutput->frameX - theoraConfigOutput->pictureX)/2)&~1;
      theoraConfigOutput->frameYOffset  = ((theoraConfigOutput->frameY - theoraConfigOutput->pictureY)/2)&~1;

      /* no reason for using another aspect ratio than 1:1, are there? */
      theoraConfigOutput->aspectRatioDenom = 1;
      theoraConfigOutput->aspectRatioNum = 1;

    }
  }

  if (wishList.changeFramerate) {

    if ( ( (theoraConfigOutput->framerateNum != wishList.framerateNum )
           || (theoraConfigOutput->framerateDenom
               != wishList.framerateDenom ) )
         && ( (theoraConfigOutput->framerateNum*1.0 )
              / (theoraConfigOutput->framerateDenom*1.0 )
              != (wishList.framerateNum*1.0 )
              / (wishList.framerateDenom*1.0 ) )) {
      theoraConfigOutput->framerateNum = wishList.framerateNum;
      theoraConfigOutput->framerateDenom = wishList.framerateDenom;
    }
  }

}

void analyseAudioTranscoding(WishList& wishList,
                             std::shared_ptr<VorbisStreamParameter> vorbisConfigInput,
                             std::shared_ptr<VorbisStreamParameter> vorbisConfigOutput)
{
  /* first we believe the output should be equal for all
   * parameters, that are not explicitly changed */
  *vorbisConfigOutput.get() = *vorbisConfigInput.get();

  if (wishList.changeAudioDatarate) {
    if (vorbisConfigOutput->datarate != wishList.audioDatarate) {
      vorbisConfigOutput->datarate = wishList.audioDatarate;
    }
  }

  if (wishList.changeAudioSamplerate) {
    if (vorbisConfigOutput->samplerate != wishList.audioSamplerate) {
      vorbisConfigOutput->samplerate = wishList.audioSamplerate;
    }

  }

  if (wishList.changeAudioChannels) {
    if (vorbisConfigOutput->channels != wishList.audioChannels) {
      vorbisConfigOutput->channels = wishList.audioChannels;
    }

  }
  return;
}

int oggTranscodeCmd(int argc, char* argv[])
{

  bool copyTheora( true);
  bool copyVorbis( true);

  WishList wishList;

  std::vector<OggComment> videoComments;

  std::vector<OggComment> audioComments;

  bool forceVideoReencode(false);
  bool forceAudioReencode(true);
  bool existenceTest(true);

  std::vector<BlendElement> blendListBefore;
  std::vector<BlendElement> blendListAfter;

  std::string programName(argv[0]);

  std::string inputFile;
  std::string outputFile;

  srand( (int) time(0) );

  int opt;
  while ( (opt = getopt(argc, argv, "hs:f:d:tD:c:C:N:F:a:A:q:p:xr:o:Q:") ) != EOF)

    switch (opt) {

    case 'h':
    case '?':
      printHelpScreen(programName);
      exit( -1);

    case 'a': {
      CmdlineExtractor::extractBlend(blendListBefore, optarg, ':', ',');
      copyTheora = false;
    }
    break;

    case 'A': {
      CmdlineExtractor::extractBlend(blendListAfter, optarg, ':', ',');
      copyTheora = false;
    }
    break;

    case 'Q': {
      uint8 _quality = boost::lexical_cast<uint8>(optarg);

      if (_quality < 1)
        _quality = 1;
      if (_quality > 5)
        _quality = 5;

      // non linear
      switch (_quality) {

      case 1:
        wishList.quality = 2;
        break;
      case 2:
        wishList.quality = 3;
        break;
      case 3:
        wishList.quality = 4;
        break;
      case 4:
        wishList.quality = 6;
        break;
      case 5:
        wishList.quality = 10;
        break;
      }

      break;
    }

    case 'q': {
      wishList.changeVideoQuality = true;
      wishList.videoQuality = boost::lexical_cast<uint32_t>(optarg);
      break;
    }

    case 's': {
      std::deque<uint32> framesize;
      CmdlineExtractor::extractUint32(framesize, optarg, 'x');
      if (framesize.size() != 2) {
        logger.error()
            << "please specify the size in the following way: -s320x480\n";
        exit( -1);
      }
      wishList.changeSize = true;
      wishList.width = framesize[0];
      wishList.height = framesize[1];

    }
    break;

    case 'f': {
      std::deque<uint32> framerate;
      CmdlineExtractor::extractUint32(framerate, optarg, ':');
      if (framerate.size() == 1) {
        wishList.changeFramerate = true;
        wishList.framerateNum = framerate[0];
        wishList.framerateDenom = 1;
        break;
      }
      if (framerate.size() == 2) {
        wishList.changeFramerate = true;
        wishList.framerateNum = framerate[0];
        wishList.framerateDenom = (framerate[1] == 0 ) ? 1
                                  : framerate[1];
        break;
      }
      logger.error()
          << "please specify the framerate in the following way -f25:2 or -f24\n";
      exit( -1);

    }
    break;

    case 'd':
      wishList.changeVideoDatarate = true;
      wishList.videoDatarate = boost::lexical_cast<uint32_t>(optarg);
      break;

    case 'D':
      wishList.changeAudioDatarate = true;
      wishList.audioDatarate = boost::lexical_cast<uint32_t>(optarg);
      break;

    case 'c':
      CmdlineExtractor::extractCommentPairs(videoComments, optarg, ';',
                                            '=');
      break;

    case 'C':
      CmdlineExtractor::extractCommentPairs(audioComments, optarg, ';',
                                            '=');
      break;

    case 'N':
      wishList.changeAudioChannels = true;
      wishList.audioChannels = boost::lexical_cast<uint32_t>(optarg);
      if ( (wishList.audioChannels != 1 )
           && (wishList.audioChannels != 2 ))
        wishList.changeAudioChannels = false;
      break;

    case 'F':
      wishList.changeAudioSamplerate = true;
      wishList.audioSamplerate = boost::lexical_cast<uint32_t>(optarg);
      break;

    case 't':
      wishList.stretch = true;
      break;

    case 'p':
      wishList.preview = boost::lexical_cast<uint32_t>(optarg);
      wishList.ignoreVorbis = true;
      break;

    case 'r':
      if (optarg[0] == 'v')
        forceVideoReencode = true;
      if (optarg[0] == 'a')
        forceAudioReencode = true;
      break;

    case 'x':
      existenceTest = false;
      break;

    case 'o':
      outputFile = std::string(optarg);
      break;
    }

  argc -= optind;
  argv += optind;

  if (existenceTest && check_file_exists(outputFile))
    exit(0);

  if (outputFile.empty() && (argc == 2)) {
    inputFile = std::string(argv[0]);
    outputFile = std::string(argv[1]);
  } else {
    if (!outputFile.empty() && (argc == 1))
      inputFile = std::string(argv[0]);
    else {
      printHelpScreen(programName);
      return ( -1 );
    }
  }

  /* create configuration */

  StreamSerializer inStream;
  if ( !inStream.open(inputFile) ) {
    logger.error() << "can not open file <"<<inputFile<<">\n";
    return ( -1 );
  }


  /* load all png pictures that should later be used */
  for (uint32 i(0); i<blendListBefore.size(); ++i) {
    try {
      RGBPlane plane;
      PictureLoader::load( plane, blendListBefore[i].getPictureName() );
      blendListBefore[i].setPicturePlane(plane);
    } catch (std::exception e) {
      logger.error() << e.what();
    }
  }

  for (uint32 i(0); i<blendListAfter.size(); ++i) {
    try {
      RGBPlane plane;
      PictureLoader::load( plane, blendListAfter[i].getPictureName() );
      blendListAfter[i].setPicturePlane(plane);
    } catch (std::exception e) {
      logger.error() << e.what();
    }
  }

  /* create special configuration for the video stream */
  VideoHook::Config videoHookConfig;

  videoHookConfig.stretch = wishList.stretch;
  videoHookConfig.quality = wishList.quality;
  videoHookConfig.preview = wishList.preview;
  videoHookConfig.trimDatarateOrQuality = false;
  videoHookConfig.blendListBefore       = blendListBefore;
  videoHookConfig.blendListAfter        = blendListAfter;


  /*  */

  /* get all information from the stream */
  std::vector<StreamConfig> demuxerInformation;

  /* read in the stream configuration that are avalable in this stream
   * The information, we need is the stream ID and the corresponding
   * stream type everything else is not needed */
  inStream.getStreamConfig(demuxerInformation);


  /* Please notice! */
  /* At this time, the stream Information are extracted from the header
   * in further analysation, we will replace these information
   * by the more accurate information created by the decoder */

  /* we create a vector for the input stream and set every
   * value to 255 (means: ignore this stream).
   * If the stream is used, the value added is the stream, where this
   * input stream should be mapped to */
  std::vector<uint8> streamMap;
  streamMap.resize(demuxerInformation.size(), 255);

  /* These are the information ordered, by the stream IDs from the input stream */
  std::vector<std::shared_ptr<HookHandler> > hookList;
  std::vector<StreamConfig> muxerInformation;

  bool foundTheora( false);
  bool foundVorbis( false);

  uint8 streamCounter( 0);

  FileRepository* repository = new FileRepository( outputFile, MediaUnit::write );

  for (uint32 i( 0); i<demuxerInformation.size(); ++i) {

    StreamConfig& decoderConfig(demuxerInformation[i]);

    if (decoderConfig.type == OggType::theora) {
      if (!foundTheora) {
        foundTheora = true;
        StreamConfig encoderConfig;

        std::shared_ptr<TheoraStreamParameter> theoraEncoderConfig;
        std::shared_ptr<TheoraStreamParameter> theoraDecoderConfig;

        std::shared_ptr<VideoHook> vHook = std::make_shared<VideoHook>(streamCounter, false, true );

        /* configure the video hook */
        vHook->configureProcess(videoHookConfig);

        hookList.push_back(vHook);

        /* We only need these information for the information printout */
        std::vector<OggComment> decoderComments;

        /* configure encoder config (StreamConfig/OggComment here) */
        vHook->setDecoderConfig(decoderConfig, decoderComments);

        /* grap the information extracted by the decoder */
        theoraDecoderConfig = std::dynamic_pointer_cast<TheoraStreamParameter>(decoderConfig.parameter);

        /* create a config for the output stream and keep a pointer */
        theoraEncoderConfig = std::make_shared<TheoraStreamParameter>();

        /* add the pointer to the configuration */
        encoderConfig.parameter = theoraEncoderConfig;

        analyseVideoTranscoding(wishList, theoraDecoderConfig,
                                theoraEncoderConfig);

        if (forceVideoReencode)
          vHook->forceReencoding();

        /* the decoder Comments are used as well in case, keepComments
         * is set within the HookHandler */
        vHook->setEncoderConfig(encoderConfig, videoComments);

        /* calculate how to handle the input, to create the correct output */
        vHook->initAndConnect();

        /* the configuration ID must match the stream ID */
        muxerInformation.push_back(encoderConfig);

        /* set the stream ID, to which this stream should be maped to */
        streamMap[i] = streamCounter;
        streamCounter++;

      } else {
        logger.warning()
            << "oggResize found more than one theora stream, only the first stream is handled\n";
      }
      continue;
    }

    if (decoderConfig.type == OggType::vorbis) {
      if (!foundVorbis) {
        StreamConfig encoderConfig;

        std::shared_ptr<VorbisStreamParameter> vorbisEncoderConfig;
        std::shared_ptr<VorbisStreamParameter> vorbisDecoderConfig;

        foundVorbis = true;

        std::shared_ptr<AudioHook> aHook = std::make_shared<AudioHook>(streamCounter, false, true );
        hookList.push_back(aHook);

        /* We only need these information for the information printout */
        std::vector<OggComment> decoderComments;

        /* configure encoder config (StreamConfig/OggComment here) */
        aHook->setDecoderConfig(decoderConfig, decoderComments);

        /* create a config for this stream */
        vorbisEncoderConfig = std::make_shared<VorbisStreamParameter>();

        /* */
        vorbisDecoderConfig
          = std::dynamic_pointer_cast<VorbisStreamParameter>(decoderConfig.parameter);

        /* */
        encoderConfig.parameter = vorbisEncoderConfig;


        analyseAudioTranscoding(wishList, vorbisDecoderConfig,
                                vorbisEncoderConfig);

        /* the decoder Comments are used as well in case, keepComments
         * is set within the HookHandler */
        aHook->setEncoderConfig(encoderConfig, videoComments);

        /* calculate how to handle the input, to create the correct output */
        aHook->initAndConnect();

        muxerInformation.push_back(encoderConfig);

        /* set the stream ID, to which this stream should be maped to */
        streamMap[i] = streamCounter;
        streamCounter++;
      } else {
        logger.warning()
            << "oggResize found more than one vorbis stream, only the first stream is handled\n";
      }
      continue;
    }

    logger.warning() << "There is actually no stream handler available to resize stream "<< decoderConfig.streamNo<<"\n";
  }

  /* configure stream hook */

  logger.info() << "Input Configuration: " << std::endl
                << "-------------------- " << std::endl;

  for (uint32 i(0); i< hookList.size(); ++i) {
    logger.info() << hookList[i]->decoderConfiguration() << std::endl;
  }

  logger.info() << "Output Configuration: " << std::endl
                << "--------------------- " << std::endl;

  for (uint32 i(0); i< hookList.size(); ++i) {
    logger.info() << hookList[i]->encoderConfiguration() << std::endl;
  }

  StreamMux streamCreate ( repository );
  streamCreate.configureStreams ( muxerInformation );
  streamCreate.recreatePacketPosition(false);

  /* run through the stream */
  OggPacket packet;
  double time;

  while (inStream.available() ) {
    time = inStream.getNextPacket(packet);

    logger.info() << "  "<<time<<"        \r";

    uint32 hookStreamID = streamMap[packet->getStreamNo()];

    if (hookStreamID == 255)
      continue;

    HookHandler& hook(*hookList[hookStreamID]);

    hook << packet;

    while (hook.available()) {
      hook >> packet;
      streamCreate << packet;
    }
  }

  /* flush all data */
  for (uint32 i(0); i<hookList.size() ; ++i) {
    hookList[i]->flush();

    while (hookList[i]->available()) {
      (*hookList[i]) >> packet;
      streamCreate << packet;
    }

  }

  /* set end of stream and do everything neccessary */
  streamCreate.setEndOfStream();

  streamCreate.close();
  inStream.close();

  logger.info() << std::endl;

  return(0);
}

int main(int argc, char* argv[])
{
  try {
    return oggTranscodeCmd(argc, argv);
  } catch (OggException & e) {
    logger.error() << "Fatal error: " << e.what() << std::endl;
    return -1;
  }
}

