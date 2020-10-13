/*
 * streamMux will multiplex a number streams to one ogg file
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
#ifndef STREAMMUX_H_
#define STREAMMUX_H_

#include <deque>
#include <vector>

#include "definition.h"
#include "oggPacket.h"
#include "oggEncoder.h"
#include "oggStreamEncoder.h"
#include "granulePosInterpreter.h"
#include "mediaRepository.h"

class MuxStreamEntry {

public:

  /* entry information */
  bool                   used;

  /* stream information */
  StreamConfig           streamConfig;
  OggStreamEncoder*      streamEncoder;
  GranulePosInterpreter* posInterpreter;

  /* packet information */
  OggPage                nextPage;
  double                 nextTime;
  bool                   empty;

  uint32                 lastPacketNo;

  /* stream buffer information */
  int                    bufferElemCounter;

  MuxStreamEntry();
  MuxStreamEntry(StreamConfig& config, OggStreamEncoder* streamEncoder, GranulePosInterpreter* posInterpreter);

  virtual ~MuxStreamEntry();

};

//! streamMux creates a new ogg media stream
/*! This object awaits a m_repository with the constructor, which informs
 *  the object about where to write the information to.
 *  Additional it awaits a vector of Stream Config information to set up
 *  the header and the granule position correctly.
 *  From that moment, the StreamMux object receives OggPackets which are
 *  placed into the media stream with the correct packaging.
 * */
class StreamMux {

protected:

  struct OutputElement {
    OggPage page;
    double  time;
    OutputElement(OggPage _page, double _time)
      : page(_page), time(_time) {}
  };

  double           m_timeOfLastPage;

  bool             m_redoTiming;

  OggEncoder       m_oggEncoder;
  MediaRepository* m_repository;

  std::vector<MuxStreamEntry> m_streamList;
  //std::list<OggPage> m_outputPageList;
  std::list<OutputElement> m_outputPageList;

  bool allBuffersEmpty();

  void writeToRepository();
  void writeToOggEncoder();

  void flushOggEncoder();

  bool findAndInsertNextPage();
  void insertHeader();
  void handleNextPage(OggPage& page, uint8 streamNo);

public:
  StreamMux(MediaRepository* repository);
  virtual ~StreamMux();

  void configureStreams(std::vector<StreamConfig>& config);
  void setEndOfStream();

  StreamMux& operator<<(OggPacket& page);

  void recreatePacketPosition(bool redoTiming);

  void close();
};

#endif /*STREAMMUX_H_*/
