/*
 * oggCat is a command line tool, to concatenate video streams
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
#include <fstream>

#include <ctime>
#include <unistd.h>

#include "definition.h"
#include "helper.h"
#include "fileRepository.h"
#include "oggPacket.h"
#include "streamMux.h"
#include "streamSerializer.h"

#include "cmdlineextractor.h"
#include "theoraStreamParameter.h"
#include "vorbisStreamParameter.h"
#include "wishList.h"
#include "hookHandler.h"
#include "videoHook.h"
#include "audioHook.h"
#include "exception.h"
#include "log.h"

void printHelpScreen(const std::string& progName)
{
  logger.error() << "usage: " << progName << " [options]"
                 << " <outputFile> <inputFile1> <inputFile2> [ <inputFile3> [ ... ] ]\n"
                 << "   or: [options] -o <outputFile> <inputFile1> <inputFile2> [ <inputFile3> [ ... ] ]\n"
                 << " -- package and version \"" << PACKAGE_STRING << "\"\n\n"
                 << "Options:\n" << "  -p    presize cutting (-pa for audio only)\n"
                 << "  -d    datarate of output stream\n"
                 << "  -q    video quality of output stream\n"
                 << "  -D    datarate of output audio stream\n"
                 << "  -Q    audio quality of output stream\n"
                 << "  -s    video size (e.g. 240x160)\n"
                 << "  -f    video framerate\n" << "  -F    audio Sample rate\n"
                 << "  -N    channel numbers\n"
                 << "  -x    no existens check for output file (for interactive usage)\n"
                 << "  -o    output file (alternative - if set, the first name is an input file!)\n"
                 << "  -rv   reencode video stream\n"<< "\n";
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

      theoraConfigOutput->calculateFrame();

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


int oggCatCmd(int argc, char* argv[])
{
  std::string programName(argv[0]);
  std::string outputFile;

  WishList wishList;

  std::vector<OggComment> videoComments;
  bool withVideoComments( false);

  std::vector<OggComment> audioComments;
  bool withAudioComments( false);

  bool reencodeVideo(false);
//  bool reencodeAudio(true); is always used

  bool existenceTest(true);

  srand((uint) time(0));

  int opt;
  while ((opt = getopt(argc, argv, "hp:d:q:o:D:s:f:F:N:tC:c:r:x")) != EOF)

    switch (opt) {

    case 'h':
    case '?':
      printHelpScreen(programName);
      exit(-1);

    case 'o':
      outputFile = std::string(optarg);
      break;

    case 'd':
      wishList.videoDatarate = CmdlineExtractor::atoi(optarg);
      wishList.changeVideoDatarate = true;
      break;

    case 'D':
      wishList.audioDatarate = CmdlineExtractor::atoi(optarg);
      wishList.changeAudioDatarate = true;
      break;

    case 'q':
      wishList.videoQuality = CmdlineExtractor::atoi(optarg);
      wishList.changeVideoQuality = true;
      break;

    case 's': {
      std::deque<uint32> framesize;
      CmdlineExtractor::extractUint32(framesize, optarg, 'x');
      if (framesize.size() != 2) {
        logger.error()
            << "please specify the size in the following way: -s320x480\n";
        exit( -1);
      }
      wishList.width = framesize[0];
      wishList.height = framesize[1];
      wishList.changeSize = true;
      break;

    }

    case 'f': {
      std::deque<uint32> framerate;
      CmdlineExtractor::extractUint32(framerate, optarg, ':');

      if (framerate.size() == 1) {
        wishList.framerateNum = framerate[0];
        wishList.framerateDenom = 1;
        wishList.changeFramerate = true;
        break;
      }

      if (framerate.size() == 2) {
        wishList.framerateNum = framerate[0];
        wishList.framerateDenom = (framerate[1] == 0 ) ? 1
                                  : framerate[1];
        wishList.changeFramerate = true;
        break;
      }
      logger.error()
          << "please specify the framerate in the following way -f25:2 or -f24\n";
      exit( -1);

      break;

    }

    case 'F':
      wishList.audioSamplerate = CmdlineExtractor::atoi(optarg);
      wishList.changeAudioSamplerate = true;
      break;

    case 'N':
      wishList.audioChannels = CmdlineExtractor::atoi(optarg);
      wishList.changeAudioChannels = true;
      break;

    case 't':
      wishList.stretch = true;
      break;

    case 'c':
      withVideoComments = true;
      CmdlineExtractor::extractCommentPairs(videoComments, optarg, ';',
                                            '=');
      break;

    case 'C':
      withAudioComments = true;
      CmdlineExtractor::extractCommentPairs(audioComments, optarg, ';',
                                            '=');
      break;

    case 'r':
      switch (optarg[0]) {
//      case 'a':
//        reencodeAudio = true;
//        break;
      case 'v':
        reencodeVideo = true;
        break;
      }
      break;

    case 'x':
      existenceTest = false;
      break;
    default:
      logger.error() << "option \"-" << opt << "\" is unknown" << std::endl;
    }

  argc -= optind;
  argv += optind;

  /* There are two possibilities to get the output file
   * "old" version is via -o option. In this case the output file is
   * not m_empty. In the other case the output file is given as the first
   * argument (except the options). */
  if (outputFile.empty()) {
    if (argc > 1) {

      outputFile = std::string(argv[0]);
      argc -= 1;
      argv += 1;

    } else {
      printHelpScreen(programName);
      exit(-1);
    }
  }

  if (existenceTest && check_file_exists(outputFile))
    exit(0);

  if (argc < 2) {
    printHelpScreen(programName);
    exit(-1);
  }

  logger.debug() << "Output file is : "<<outputFile<<" next file is "<<argv[0]<<std::endl;

  std::string baseFile(argv[0]);

  /* open the first file to be read */
  std::vector<StreamConfig> originalConfigList;
  StreamSerializer* serializer = new StreamSerializer;

  if (!serializer->open(baseFile)) {
    logger.error() << "Can not open file <" << baseFile << ">\n";
    exit(-1);
  }

  /* read the stream configuration */
  serializer->getStreamConfig(originalConfigList);

  /* we create a vector for the input stream and set every
   * value to 255 (means: ignore this stream).
   * If the stream is used, the value added is the stream, where this
   * input stream should be mapped to */
  std::vector<uint8> streamMap;
  streamMap.resize(originalConfigList.size(), 255);

  /* These are the information ordered, by the stream IDs from the input stream */
  std::vector<std::shared_ptr<HookHandler> > hookList;
  std::vector<StreamConfig> muxerInformation;

  bool foundTheora(false);
  bool foundVorbis(false);

  uint8 streamCounter( 0);
  uint32 startInputfiles(1);
  /*  create the first resize round  */

  for (uint32 i=0; i<originalConfigList.size(); ++i) {

    StreamConfig& decoderConfig(originalConfigList[i]);

    if (decoderConfig.type == OggType::theora) {
      if (!foundTheora) {
        foundTheora = true;
        StreamConfig encoderConfig;

        std::shared_ptr<TheoraStreamParameter> theoraEncoderConfig;
        std::shared_ptr<TheoraStreamParameter> theoraDecoderConfig;

        std::shared_ptr<VideoHook> vHook = std::make_shared<VideoHook>(streamCounter, false, true );

        if (reencodeVideo)
          vHook->forceReencoding();

        /* here, we configure things, that do not change
         * the transcoding process (alpha blend etc) */
        VideoHook::Config videoHookConfig;
        videoHookConfig.stretch = wishList.stretch;

        /* configure the video hook */
        vHook->configureProcess(videoHookConfig);

        hookList.push_back(vHook);

        /* We only need these information for the information printout */
        std::vector<OggComment> decoderComments;

        /* configure encoder config (StreamConfig/OggComment here) */
        vHook->setDecoderConfig(decoderConfig, decoderComments);

        /* grap the information extracted by the decoder */
        theoraDecoderConfig
          = std::dynamic_pointer_cast<TheoraStreamParameter>(decoderConfig.parameter);

        /* create a config for the output stream and keep a pointer */
        theoraEncoderConfig = std::make_shared<TheoraStreamParameter>();

        analyseVideoTranscoding(wishList, theoraDecoderConfig,
                                theoraEncoderConfig);

        if (reencodeVideo)
          theoraEncoderConfig->calculateFrame();

        if (!withVideoComments)
          videoComments = decoderComments;

        /* add the pointer to the configuration */
        encoderConfig.parameter = theoraEncoderConfig;

        /* the decoder Comments are used as well in case, keepComments
         * is set within the HookHandler */
        vHook->setEncoderConfig(encoderConfig, videoComments);

        /* the configuration ID must match the stream ID */
        muxerInformation.push_back(encoderConfig);

        /* calculate how to handle the input, to create the correct output */
        vHook->initAndConnect();

        /* set the stream ID, to which this stream should be maped to */
        streamMap[i] = streamCounter;
//        theoraStreamID = streamCounter;

        streamCounter++;

      } else {
        logger.warning()
            << "oggCat found more than one theora stream, only the first stream is handled\n";
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
        vorbisDecoderConfig = std::dynamic_pointer_cast<VorbisStreamParameter>(decoderConfig.parameter);

        /* */
        encoderConfig.parameter = vorbisEncoderConfig;

        analyseAudioTranscoding(wishList, vorbisDecoderConfig,
                                vorbisEncoderConfig);

        if (!withAudioComments)
          audioComments = decoderComments;

        /* the decoder Comments are used as well in case, keepComments
         * is set within the HookHandler */
        aHook->setEncoderConfig(encoderConfig, audioComments);

        /* calculate how to handle the input, to create the correct output */
        aHook->initAndConnect();

        muxerInformation.push_back(encoderConfig);

        /* set the stream ID, to which this stream should be maped to */
        streamMap[i] = streamCounter;
//        vorbisStreamID = streamCounter;

        streamCounter++;
      } else {
        logger.warning()
            << "oggCat found more than one vorbis stream, only the first stream is handled\n";
      }
      continue;
    }

//    logger.error()
//        << "There is actually no stream handler available to resize this stream \n";
  }

  logger.info() << "Output Configuration: " << std::endl
                << "--------------------- " << std::endl;

  for (uint32 i(0); i< hookList.size(); ++i) {
    logger.info() << hookList[i]->encoderConfiguration() << std::endl;
  }

  logger.info() << "Mapping\n";
  for (uint32 i(0); i<streamMap.size(); ++i) {
    if (streamMap[i] == 255)
      logger.info() << " Input Stream "<<i<< " is not used"<<std::endl;
    else
      logger.info() << " Input Stream "<<i<< " maps to output stream "<< (uint32)streamMap[i]<<std::endl;
  }

  //++++++++++++++++++++++++++++

  FileRepository* repository (0);
  try {
    repository = new FileRepository( outputFile, MediaUnit::write );
  } catch (std::exception e) {
    logger.error() << e.what() << std::endl;
    exit(-1);
  }

  StreamMux streamCreate(repository);
  streamCreate.configureStreams(muxerInformation);
  streamCreate.recreatePacketPosition(false);

  /* run through the stream */
  OggPacket packet;
  double time;

  while (serializer->available() ) {
    time = serializer->getNextPacket(packet);

    logger.info() << "  " << time << "      \r";

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

  logger.info() << "\n";
  // end of the road
  delete serializer;

  // handle the other files given with the arguments list
  for (uint32 j(startInputfiles); j<(uint32)argc; ++j) {

    StreamSerializer serializer;

    /* try to open the file. If file is not available switch to the next one */
    std::string filename(argv[j]);
    if (!serializer.open(filename)) {
      logger.error() << "Can not open file <" << filename << ">\n";
      continue;
    }

    logger.info() << "Concatenating file <"<<filename<<">"<<std::endl;

    uint32 newStreamCounter(0);

    std::vector<StreamConfig> ConfigList;

    /* read the stream configuration of the actual file */
    serializer.getStreamConfig(ConfigList);

    foundTheora = false;
    foundVorbis = false;

    /* create a new stream Map */
    streamMap.clear();
    streamMap.resize(ConfigList.size(), 255);

    for (uint32 l( 0); l<ConfigList.size(); ++l) {

      StreamConfig& decoderConfig(ConfigList[l]);

      /* of the actual stream is a theora stream investigate more */
      if (decoderConfig.type == OggType::theora) {
        /* run through the stream map to find the original stream,
         * this stream should be mapped to */
        for (uint32 k(0); k<hookList.size(); ++k) {
          if ((hookList[k]->getType() == OggType::theora) && !foundTheora) {
            foundTheora = true;
//            logger.debug() << "Theora found ("<< l <<") -> "<<k<<std::endl;
            VideoHook& vHook = dynamic_cast<VideoHook&>(*hookList[k]);

            /* We only need these information for the information printout */
            std::vector<OggComment> decoderComments;

            /* configure encoder config (StreamConfig/OggComment here) */
            vHook.setDecoderConfig(decoderConfig, decoderComments);
            /* as not every stream is de- and encoded, the encoder
             * may be confused */
            vHook.resetEncoder();

            /* calculate how to handle the input, to create the correct output */
            vHook.initAndConnect();

            /* set the stream ID, to which this stream should be maped to */
            streamMap[l] = (uint8)k;

            newStreamCounter++;

          }
          continue;
        }
      }

      if (decoderConfig.type == OggType::vorbis) {
        /* run through the stream map to find the original stream,
         * this stream should be mapped to */
        for (uint32 k(0); k<hookList.size(); ++k) {
          if ((hookList[k]->getType() == OggType::vorbis) && !foundVorbis) {
            foundVorbis = true;
//            logger.debug() << "Vorbis found ("<< l <<") -> "<<k<<std::endl;
            AudioHook& aHook = dynamic_cast<AudioHook&>(*hookList[k]);

            /* We only need these information for the information printout */
            std::vector<OggComment> decoderComments;

            /* configure encoder config (StreamConfig/OggComment here) */
            aHook.setDecoderConfig(decoderConfig, decoderComments);

            /* calculate how to handle the input, to create the correct output */
            aHook.initAndConnect();

            /* set the stream ID, to which this stream should be maped to */
            streamMap[l] = (uint8)k;

            newStreamCounter++;

          }
          continue;
        }

//        logger.error()
//            << "There is actually no stream handler available to resize this stream \n";
      }
    }

    if (streamCounter != newStreamCounter) {
      logger.error() << "File <"<<argv[j]
                     <<"> does not carry enough streams\n";
      continue;
    }


    logger.info() << "Mapping\n";
    for (uint32 i(0); i<streamMap.size(); ++i) {
      if (streamMap[i] == 255)
        logger.info() << " Input Stream "<<i<< " is not used"<<std::endl;
      else
        logger.info() << " Input Stream "<<i<< " maps to output stream "<< (uint32)streamMap[i]<<std::endl;
    }


    while (serializer.available()) {
      time = serializer.getNextPacket(packet);

      logger.info() << "  " << time << "        \r";

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

    serializer.close();

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

  logger.info() << std::endl;

  return (0);
}

int main(int argc, char* argv[])
{
  try {
    return oggCatCmd(argc, argv);
  } catch (OggException & e) {
    logger.error() << "Fatal error: " << e.what() << std::endl;
    return -1;
  }
}

