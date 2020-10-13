/*
 * OggEncoder
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

#ifndef OGG_ENCODER_H
#define OGG_ENCODER_H

#include <list>
#include <vector>

#include "mediaEncoder.h"
#include "oggPage.h"
#include "rawMediaPacket.h"
#include "granulePosInterpreter.h"
#include "streamConfig.h"

class PageBufferElement {

public:
  PageBufferElement();
  virtual ~PageBufferElement();

  double                 time;
  bool                   empty;
  OggPage                page;
  GranulePosInterpreter* interpreter;

};

class OggEncoder : public MediaEncoder {

protected:
  bool                           withBuffer;
  std::vector<PageBufferElement> oggBuffer;
  std::list<RawMediaPacket>      rawPacketList;

  void serializePage(OggPage& page);
  void insertNextPage(OggPage& page);
  void createJitterBuffer(uint8 numOfStreams);

public:

  OggEncoder();
  virtual ~OggEncoder();

  void configure(std::vector<StreamConfig*> configInformation);

  void flush();

  OggEncoder& operator<<(OggPage page);
  OggEncoder& operator>>(RawMediaPacket& packet);
};

#endif
