/*
 * oggSilence is a command line tool, to create silence vorbis files
 *
 * Copyright (C) 2009 Joern Seger
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

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

#include "vorbisEncoder.h"
#include "streamMux.h"
#include "fileRepository.h"
#include "oggComment.h"
#include "cmdlineextractor.h"
#include "exception.h"
#include "log.h"

#define BUNCHSIZE 512

void printHelpScreen(std::string& name)
{
  logger.error() << "usage "<<name<< " -d <datarate in bit/s> -n <number of channels> -r <sample rate in Hz> -l <length in ms> <output file>\n";
}

AudioPacket getSilencePacket(uint32 channels, uint32 length)
{
  float silence[length];

  for (uint32 i(0); i<length; ++i)
    silence[i] = 0.0;

  AudioPacketInternal* internal = new AudioPacketInternal(channels,length);

  for (uint32 j(0); j<channels; ++j)
    internal->setDataOfChannel(j, silence);

  // funny stack stuff ;-)
  return(AudioPacket(internal));
}

int oggSilenceCmd( int argc, char* argv[] )
{

  VorbisEncoder encoder(0);
  AudioPacket   audioPacket;
  AudioPacket   lastAudioPacket;

  uint32 samplerate(44100);
  uint32 channels(2);
  uint32 datarate(64000);
  uint32 length(60000);      // 1 minute

  /* Initialisation */

  std::string outputFile;
  std::string programName(argv[0]);

  srand(time(0));

  int opt;
  while ((opt = getopt(argc, argv, "ho:d:n:r:l:")) != EOF)

    switch (opt) {

    case 'h':
      printHelpScreen(programName);
      exit(-1);

    case 'd':
      datarate = CmdlineExtractor::atoi(optarg);
      break;

    case 'o':
      outputFile = std::string(optarg);
      break;

    case 'n':
      channels = CmdlineExtractor::atoi(optarg);
      break;

    case 'r':
      samplerate = CmdlineExtractor::atoi(optarg);
      break;

    case 'l':
      length = CmdlineExtractor::atoi(optarg);
      break;

    }

  argc -= optind;
  argv += optind;

  if ((argc > 1)) {
    printHelpScreen(programName);
    exit (-1);
  }

  if (argc > 0) {
    outputFile = std::string(argv[0]);
  }

  /* Handle wrong parameter and parameter combinations */
  if (outputFile.empty()) {
    printHelpScreen(programName);
    exit (-1);
  }

  StreamMux muxer(new FileRepository(outputFile, MediaUnit::write));

  muxer.recreatePacketPosition(false);

  /* configure encoder */
  std::shared_ptr<VorbisStreamParameter> config = std::make_shared<VorbisStreamParameter>();
  config->datarate = datarate;
  config->channels = channels;
  config->samplerate = samplerate;

  StreamConfig streamConfig;
  streamConfig.parameter = config;

  std::vector<OggComment> comments; // none

  try {
    encoder.configureEncoder(streamConfig, comments);
  } catch (std::exception & e) {
    logger.error() << e.what() << std::endl;
    exit(-1);
  } catch (...) {
    //logger.error() << what();
    exit(-1);
  }

  logger.error() << "Creating ogg file with the following parameters\n"<<streamConfig.parameter->toString();

  /* there is only one stream in this file */
  std::vector<StreamConfig> configList;
  configList.push_back(streamConfig);

  /* configure the muxer */
  muxer.configureStreams(configList);

  uint32 completeSamples((float)length/1000.0*samplerate);

  /* create one silence packet */
  audioPacket = getSilencePacket(channels, BUNCHSIZE);

  if (completeSamples%BUNCHSIZE != 0) {
    lastAudioPacket = getSilencePacket(channels, completeSamples%BUNCHSIZE);
  }

  OggPacket packet;

  for (uint32 i(0); i<completeSamples/BUNCHSIZE; ++i) {

    logger.debug() <<(i+1)*BUNCHSIZE<<"\r";

    encoder << audioPacket;
    if (encoder.isAvailable()) {
      encoder >> packet;
      muxer << packet;
    }
  }

  logger.debug() << "\n";

  if (completeSamples%BUNCHSIZE != 0) {
    logger.debug() << "\nwrite last frame with "<<completeSamples%BUNCHSIZE<<std::endl;
    encoder << lastAudioPacket;
  }

  encoder.flush();

  while (encoder.isAvailable()) {
    encoder >> packet;
    muxer << packet;
  }

  muxer.setEndOfStream();
  muxer.close();

  return(0);
}

int main(int argc, char* argv[])
{
  try {
    return oggSilenceCmd(argc, argv);
  } catch (OggException & e) {
    logger.error() << "Fatal error: " << e.what() << std::endl;
    return -1;
  }
}

