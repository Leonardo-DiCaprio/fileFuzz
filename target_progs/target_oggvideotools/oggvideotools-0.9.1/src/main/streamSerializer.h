/*
 * streamSerialize will output a serialized stream of packets from a file
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

#ifndef STREAMSERIALIZER_H_
#define STREAMSERIALIZER_H_

#include<map>
#include<vector>

#include "definition.h"
#include "mediaRepository.h"
#include "oggDecoder.h"
#include "oggStreamDecoder.h"
#include "granulePosInterpreter.h"
#include "streamConfig.h"

class StreamEntry {

public:
  StreamConfig           streamConfig;
  OggStreamDecoder*      streamDecoder;
  GranulePosInterpreter* posInterpreter;

  OggPacket              nextPacket;
  double                 nextTime;
  bool                   endOfStream;
  bool                   empty;

  StreamEntry();
  StreamEntry(StreamConfig& config, OggStreamDecoder* sDecoder);
  virtual ~StreamEntry();

  bool allHeadersCollected();
};

//! class to reserialize ogg packets
/* reserializing an ogg stream is not as easy as it seems:
 * you always */
class StreamSerializer {

protected:
  enum InitStates {
    created,
    reposOpened,
    initialized
  };

  InitStates       initState;
  MediaRepository* repository;
  OggDecoder*      oggDecoder;
  std::map<uint32, StreamEntry> streamList;

  uint32 streamEndCounter;

  bool fillPage();
  bool fillStreams();
  bool extractStreams();

  void insertNextPacket(StreamEntry& entry);

  // none copieable serializer
  StreamSerializer(const StreamSerializer& streamSerializer) {}

public:
  StreamSerializer();
  virtual ~StreamSerializer();

  void getStreamConfig(std::vector<StreamConfig>& configList);

  bool available();

  bool open(std::string& datasource);
  bool open(MediaRepository* _repository);
  void close();

  double getNextPacket(OggPacket& packet);

};

#endif /*STREAMSERIALIZER_H_*/
