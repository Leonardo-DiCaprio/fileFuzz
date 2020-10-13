/*
 * theoraVideoPacket
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
#ifndef THEORAVIDEOPACKET_H_
#define THEORAVIDEOPACKET_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#warning only use with autotools
#endif

#ifdef HAVE_LIBTHEORA

#include <theora/theora.h>
#include "refObject.h"
#include "definition.h"

class VideoData {

public:

  yuv_buffer yuvBuffer;

  uint64     granulePosition;
  double     time;
  bool       keyframe;

};

class TheoraVideoPacket : public RefObject<VideoData> {

public:

  TheoraVideoPacket();
  TheoraVideoPacket(TheoraVideoPacket& packet);
  virtual ~TheoraVideoPacket();

  TheoraVideoPacket& operator=(const TheoraVideoPacket& packet);

  uint64      getGranulePosition();
  double      getTime();
  bool        isKeyFrame();
  yuv_buffer& getYuvBuffer();

  void        setGranulePosition(uint64 position);
  void        setTime(double picTime);
  void        setKeyFrame(bool isFrame);
  void        setYuvBuffer(yuv_buffer& buffer);

};

#endif

#endif /*THEORAVIDEOPACKET_H_*/
