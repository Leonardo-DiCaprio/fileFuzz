/*
 * oggStreamDecoder is a class to extract an ogg packet from an
 * ogg page stream
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

#ifndef OGGSTREAMDECODER_H_
#define OGGSTREAMDECODER_H_

#include<list>

#include "mediaDecoder.h"
#include "oggPage.h"
#include "oggPacket.h"
#include "oggTypes.h"

/* TODO: Exception Handling
 * The library should provide a consistent exception handling */

class OggStreamDecoder : public MediaDecoder {

protected:

  /*! A segment element, to cut the page into it's pieces */
  struct SegmentElement {
    uint8* data;
    uint32 length;
    SegmentElement(uint8* data=0, uint32 length=0);
  };

  uint32                 m_packetCount;
  uint32                 m_serialNo;
  SegmentElement         m_tmpSegment;
  std::list<OggPacket>   m_oggPacketList;

  void init(OggPage page);

public:
  OggStreamDecoder();
  virtual ~OggStreamDecoder();

  virtual OggStreamDecoder& operator<<(OggPage& page);
  virtual OggStreamDecoder& operator>>(OggPacket& packet);

  OggPacket inspectNextPacket();

  uint32 getSerialNo();

  virtual void clear();
};

#endif /*OGGSTREAMDECODER_H_*/
