/*
 * oggSlideshow creates a slideshow from a number of pictures
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
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <ctime>
#include <exception>
#include <memory>
#include <unistd.h>

#include "th_helper.h"

#include "definition.h"
#include "theoraEncoder.h"
#include "fileRepository.h"
#include "streamMux.h"
#include "cmdlineextractor.h"

#include "effector.h"
#include "effectorTypes.h"
#include "crossfader.h"
#include "kenburnseffect.h"
#include "lowpassEffect.h"
#include "plainPicture.h"
#include "shiftEffect.h"
#include "shiftblendEffect.h"

#include "pictureLoader.h"
#include "pictureResize.h"

#include "log.h"
#include "exception.h"

void printHelpScreen(std::string& name)
{
  std::cerr << "usage: "<< name <<" [options] <picture1.bmp> <picture2.bmp> \n";
  std::cerr << "Options: \n"
            << " -s <width>x<height>: picture width/height of the output frame\n"
            << " -f <frames/s>      : frames per second\n"
            << " -o <output file>   : name of the output file\n"
            << " -l <length>        : number of frames per picture frequence\n"
            << " -d <datarate>      : datarate in bit/second\n"
            << " -r <resample>      : resizes the original pictures to video frame width/height and the additional resample factor\n"
            << " -e                 : reframe picture\n"
            << " -t <type>          : kb - Ken Burns\n"
            << "                      cf - cross fade\n"
            << "                      p  - plain\n"
            << "                      bl - blur\n"
            << "                      s  - shift\n"
            << "                      b  - shiftblend\n"
            << " -q <quality>       : quality (0-63)\n"
            << " -c                 : comments in form type=value;type=value\n";

  std::cerr << "\nadditionally you are able to set a prefix walk with -tkb:<prefixNum>\n";
}

//int main(int argc, char* argv[])
int oggSlideshowCmd(int argc, char* argv[])
{
  /* default values */
  uint32 width(480);
  uint32 height(320);
  uint32 framesPerSecond(24);
  std::string outputFile("slideshow.ogv");
  uint32 datarate(0);
  uint32 quality(32);
  float resample(1.4);
  bool reframe(false);
  std::vector<OggComment> oggComments;
  int32 predefine(0);
  SlideshowElement defaultSlide;
  defaultSlide.duration = 8;
  defaultSlide.type = KenBurns;

  srand(time(0));

  std::string programName(argv[0]);

  int opt;
  while ((opt = getopt(argc, argv, "hp:f:o:l:d:r:t:s:ec:q:")) != EOF)

    switch (opt) {

    case 'h':
    case '?':
      printHelpScreen(programName);
      exit(-1);

    case 's': {
      std::deque<uint32> framesize;
      CmdlineExtractor::extractUint32(framesize, optarg, 'x');
      if (framesize.size() != 2) {
        logger.error() << "please specify the size in the following way: -s320x480\n";
        exit(-1);
      }
      width = framesize[0];
      height = framesize[1];

    }
    break;

    case 'q':
      quality = atoi(optarg);
      break;

    case 'f':
      framesPerSecond = atoi(optarg);
      break;

    case 'o':
      outputFile = std::string(optarg);
      break;

    case 'l':
      defaultSlide.duration = atof(optarg);
      break;

    case 'd':
      datarate = atoi(optarg);
      break;

    case 'r':
      resample = atof(optarg);
      if ((resample < 1) || (resample > 2))
        resample = 1.2;
      break;

    case 'e': {
      logger.debug() << "reframing\n";
      reframe = true;
      break;
    }

    case 't': {

      std::string typeStr;
      std::string teststring(optarg);
      std::stringstream tmp;

      std::string::size_type pos = teststring.find(':');
      typeStr = teststring.substr(0,pos);
      if ((pos != std::string::npos) && (pos+1 < teststring.size())) {
        tmp << teststring.substr(pos+1, std::string::npos);
        tmp >> predefine;
        std::cerr << "Predefine: "<< predefine<<std::endl;
      }

      if ((typeStr == "kb") || (typeStr =="KenBurns")|| (typeStr == "KB")) {

        defaultSlide.type = KenBurns;
        break;
      }
      if ((typeStr == "cf") || (typeStr =="crossfade")) {
        defaultSlide.type = Crossfade;
        break;
      }

      if ((typeStr == "p") || (typeStr =="plain")|| (typeStr == "simple")) {
        defaultSlide.type = Plain;
        break;
      }

      if ((typeStr == "b") || (typeStr =="bl") || (typeStr == "blur") ||
          (typeStr == "lp") || (typeStr == "lowpass")) {
        defaultSlide.type = Blur;
        break;
      }

      if ((typeStr == "s") || (typeStr =="sh") || (typeStr == "shift")) {
        defaultSlide.type = Shift;
        break;
      }

      if ((typeStr =="sb") || (typeStr == "shiftlend")) {
        defaultSlide.type = ShiftBlend;
        break;
      }

      std::cerr << "Unknown Type: (" << typeStr << ") using Ken Burns";
      defaultSlide.type = KenBurns;
    }
    break;

    case 'c': {
      CmdlineExtractor::extractCommentPairs ( oggComments, optarg, ';', '=' );

    }


    }

  argc -= optind;
  argv += optind;

  if ((argc < 1)) {
    printHelpScreen(programName);
    return (-1);
  }

  StreamConfig streamConf;
  std::shared_ptr<TheoraStreamParameter> config = std::make_shared<TheoraStreamParameter>();
  streamConf.parameter = config;

// for valgrind
#ifdef HAVE_BZERO
//  bzero(config,sizeof(TheoraStreamParameter));
#else
//  memset(config, 0x00, sizeof(TheoraStreamParameter));
#endif

  /* create configuration */
  config->pictureX         = width;
  config->pictureY         = height;
  config->calculateFrame();
  config->aspectRatioDenom = 1;
  config->aspectRatioNum   = 1;
  config->framerateNum     = framesPerSecond;
  config->framerateDenom   = 1;
  config->keyframeShift    = 6;
  config->pixel_fmt        = TheoraStreamParameter::pf_420;//TheoraStreamParameter::pf_444;
  config->colorspace       = TheoraStreamParameter::unspecified;

  config->videoBitrate = datarate;
  config->videoQuality = quality;

  /* create stream configuration */
  TheoraEncoder theoraEncoder(0);

  /* configure the theora encoder and get a stream config back
   * which configures the stream multiplexer */
  try {
    theoraEncoder.configureEncoder(streamConf, oggComments);
  } catch (std::exception e) {
    std::cerr << e.what();
    exit(-1);
  }

  // encoder might want another frame size:

  std::vector<StreamConfig> configList;
  configList.push_back(streamConf);

  /* create a m_repository, where the data should be placed */
  FileRepository* repository = new FileRepository(outputFile, MediaUnit::write);

  /* create a stream multiplexer */
  StreamMux streamCreate(repository);

  /* configure the stream multiplexer */
  streamCreate.configureStreams(configList);

  /* extract the RGB picture plane */
  RGBPlane pictureRGB;

  std::shared_ptr<Effector> effector;

  bool first(true);
  bool noneFound(true);
  // run through all pictures in command line
  for (int32 i(0); i<argc; ++i) {

    bool last = (i == (argc-1));
    try {

      // initialize the slide specification with default values
      SlideshowElement slide(defaultSlide);

      // extract the actual slide specifications from the next argument
      CmdlineExtractor::extractSlideshow(argv[i],',',slide);

      logger.info() << "\ncreating video stream for picture <"
                    << slide.filename << ">\n";

      // extract parameters

      uint32 loadWidth;
      uint32 loadHeight;

      if (slide.type == KenBurns) {
        loadWidth  = (uint32)(width*resample);
        loadHeight = (uint32)(height*resample);
      } else {
        loadWidth = width;
        loadHeight = height;
      }

      bool biggest = (!reframe);
      if (PictureLoader::load(pictureRGB, slide.filename, loadWidth, loadHeight, biggest) == false) {
        continue;
      }
      noneFound = false;

      /* add borders, if aspect ratio does not match and the user wants that */
      if (reframe && ((loadWidth != pictureRGB->width) || (loadHeight != pictureRGB->height))) {
        logger.info() << "Picture aspect ratio does not match, doing reframing\n";
        pictureRGB = PictureResize::reframe(pictureRGB, loadWidth, loadHeight);
      }

      /* configure the effector */
      switch (slide.type) {

      case KenBurns: {

        KenBurnsEffect::KenBurnsConfig config;
        if (predefine == 0)
          config = KenBurnsEffect::createKBconfigRandom(pictureRGB, loadWidth, loadHeight, width, height, slide.duration*framesPerSecond, framesPerSecond);
        else if (predefine < 0) {
          config.startpointX = slide.startPosX;
          config.startpointY = slide.startPosY;
          config.endpointX = slide.endPosX;
          config.endpointY = slide.endPosY;
          config.zoomStart = slide.startZoom;
          config.zoomEnd = slide.endZoom;
          config.sequenceLength = slide.duration * framesPerSecond;
          config.blindLength = framesPerSecond;
          config.origPlane = pictureRGB;
          config.outputWidth = width;
          config.outputHeight = height;

          std::cerr << "s:" << slide.startPosX<<":"<<slide.startPosY<<" -> "<<slide.endPosX<<":"<<slide.endPosY<<"\n";
        } else
          config = KenBurnsEffect::createKBconfigPredefine(pictureRGB, loadWidth, loadHeight, width, height, slide.duration*framesPerSecond, framesPerSecond, predefine);


        config.first = first;
        config.last = last;

        if (!effector.get() || GetEffectorType()(*effector) != KenBurns) {
          effector.reset(new KenBurnsEffect);
        }
        static_cast<KenBurnsEffect*>(effector.get())->configure(config);

        break;
      }


      case Crossfade: {

        Crossfader::CrossfaderConfig config;

        config.origPlane      = pictureRGB;
        config.blindLength    = framesPerSecond;
        config.sequenceLength = slide.duration*framesPerSecond;
        config.outputWidth    = width;
        config.outputHeight   = height;
        config.first          = first;

        if (!effector.get() || GetEffectorType()(*effector) != Crossfade) {
          effector.reset(new Crossfader);
        }
        static_cast<Crossfader*>(effector.get())->configure(config);

        break;
      }

      case Shift: {

        ShiftEffect::ShiftConfig config;

        config.origPlane      = pictureRGB;
        config.blindLength    = framesPerSecond;
        config.sequenceLength = slide.duration*framesPerSecond;
        config.outputWidth    = width;
        config.outputHeight   = height;
        config.first          = first;

        if (!effector.get() || GetEffectorType()(*effector) != Shift) {
          effector.reset(new ShiftEffect);
        }
        static_cast<ShiftEffect*>(effector.get())->configure(config);

        break;
      }

      case ShiftBlend: {

        ShiftblendEffect::ShiftConfig config;

        config.origPlane      = pictureRGB;
        config.blindLength    = framesPerSecond;
        config.sequenceLength = slide.duration*framesPerSecond;
        config.outputWidth    = width;
        config.outputHeight   = height;
        config.first          = first;
        config.type           = ShiftblendEffect::ShiftConfig::Right;

        if (!effector.get() || GetEffectorType()(*effector) != ShiftBlend) {
          effector.reset(new ShiftblendEffect);
        }
        static_cast<ShiftblendEffect*>(effector.get())->configure(config);

        break;
      }

      case Plain: {

        PlainPicture::PlainPictureConfig config;

        config.origPlane      = pictureRGB;
        config.sequenceLength = slide.duration*framesPerSecond;
        config.outputWidth    = width;
        config.outputHeight   = height;

        if (!effector.get() || GetEffectorType()(*effector) != Plain) {
          effector.reset(new PlainPicture);
        }
        static_cast<PlainPicture*>(effector.get())->configure(config);

        break;
      }

      case Blur : {

        LowpassEffect::LowPassPictureConfig config;

        config.origPlane = pictureRGB;
        config.blindLength = framesPerSecond;
        config.sequenceLength = slide.duration*framesPerSecond;
        config.outputWidth = width;
        config.outputHeight = height;
        config.first = first;
        config.last = last;

        if (!effector.get() || GetEffectorType()(*effector) != Blur) {
          effector.reset(new LowpassEffect);
        }
        static_cast<LowpassEffect*>(effector.get())->configure(config);
        break;
      }
      }

      RGBPlane        outputPlane;
      OggPacket       packet;
      th_ycbcr_buffer theoraPictureBuffer;
      th_clean_ycbcr(theoraPictureBuffer);

      while (effector->available()) {

        (*effector) >> outputPlane;

        PictureLoader::exportYCrCb_theora(outputPlane, theoraPictureBuffer);

        theoraEncoder << theoraPictureBuffer;
        theoraEncoder >> packet;
        std::cerr << "\r " <<std::fixed << packet->getPacketNo()*1.0/(framesPerSecond*1.0)<<"               ";
        streamCreate << packet;

      }

      th_free_ycbcr(theoraPictureBuffer);

    } catch (const char* errorString) {
      std::cout << errorString << std::endl;
      return(-1);
    }
    first = false;
  }

  if (noneFound)
    return(-1);

  streamCreate.setEndOfStream();
  streamCreate.close();

  std::cout << std::endl;
#ifdef OSX_MALLOC_DEBUG
  std::cout << "Done!\n";
  while (1==1) { }
#endif

  return(0);
}

int main(int argc, char* argv[])
{
  logger.setLevel(OggLog::LOG_INFO);
  try {
    return oggSlideshowCmd(argc, argv);
  } catch (OggException & e) {
    logger.error() << "Fatal error: " << e.what() << std::endl;
    return (-1);
  }
}
