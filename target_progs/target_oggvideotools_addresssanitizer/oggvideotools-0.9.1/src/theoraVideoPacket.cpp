/*
 * TheoraVideoPacket
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

#include "theoraVideoPacket.h"

#ifdef HAVE_LIBTHEORA

TheoraVideoPacket::TheoraVideoPacket()
{}

TheoraVideoPacket::TheoraVideoPacket(TheoraVideoPacket& packet)
  : RefObject<VideoData>(packet)
{}

TheoraVideoPacket::~TheoraVideoPacket()
{}

uint64 TheoraVideoPacket::getGranulePosition()
{
  if (objPtr)
    return (objPtr->granulePosition);

  return(0);
}

double TheoraVideoPacket::getTime()
{
  if (objPtr)
    return (objPtr->time);

  return(0);
}

bool TheoraVideoPacket::isKeyFrame()
{
  if (objPtr)
    return (objPtr->keyframe);

  return(0);
}

yuv_buffer& TheoraVideoPacket::getYuvBuffer()
{
  if (objPtr)
    return (objPtr->yuvBuffer);

  static yuv_buffer dummy;

  return(dummy);
}

void TheoraVideoPacket::setGranulePosition(uint64 position)
{
  if (objPtr)
    objPtr->granulePosition = position;
}

void TheoraVideoPacket::setTime(double picTime)
{
  if (objPtr)
    objPtr->time = picTime;
}

void TheoraVideoPacket::setKeyFrame(bool isKFrame)
{
  if (objPtr)
    objPtr->keyframe = isKFrame;
}

void TheoraVideoPacket::setYuvBuffer(yuv_buffer& buffer)
{
  if (objPtr)
    objPtr->yuvBuffer = buffer;
}

#endif
