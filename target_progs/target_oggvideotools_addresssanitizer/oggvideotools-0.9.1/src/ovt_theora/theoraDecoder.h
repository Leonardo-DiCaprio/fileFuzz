/*
 * TheoraDecoder
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

#ifndef THEORADECODER_H_
#define THEORADECODER_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#warning only use with autotools
#endif

#ifdef HAVE_LIBTHEORADEC

#include <list>
#include <vector>
#include <theora/codec.h>
#include <theora/theoradec.h>
#include "definition.h"

#include "mediaOutputDecoder.h"
#include "theoraStreamParameter.h"

class TheoraDecoder : public MediaOutputDecoder {

protected:

  th_info        theoraInfo;
  th_comment     theoraComment;
  th_setup_info* setupInfo;

  th_dec_ctx*    theoraDecState;

  std::list<OggPacket>   packetList;
//  std::vector<OggPacket> headerList;

  uint8 initCount;
  uint32 packetCount;

  static const uint32 maxVideoPlaneSize = (4096*4096);

  void reportTheoraError(int error) const;

public:
  TheoraDecoder(uint8 streamID = 0);
  virtual ~TheoraDecoder();

  virtual void initDecoder(StreamConfig& config, std::vector<OggComment>& commentList);

  virtual MediaOutputDecoder& operator<<(OggPacket packet);

  TheoraDecoder& operator>>(th_ycbcr_buffer& picture);


  th_info&        getInfo();
  th_comment&     getComment();

  bool            isNextPacketKeyframe();
  uint32          getPositionOfNextPacket();

  virtual void clear();

  uint32          getWidth();
  uint32          getHeight();

  static bool     isPacketKeyframe(OggPacket packet);

  virtual std::string configuration() const;


};

#endif // WITH_LIBTHEORA

#endif /*THEORADECODER_H_*/
