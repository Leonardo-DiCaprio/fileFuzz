/*
 * RawMediaPacket class to carry a raw bunch of data
 *
 * Copyright (C) 2005-2008 Joern Seger
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

#ifndef RAWMEDIAPACKET_H_
#define RAWMEDIAPACKET_H_

#include <memory>
#include <vector>
#include <cstdint>

#include "definition.h"
//#include "refObject.h"

class RawMediaPacketInternal;
typedef std::shared_ptr<RawMediaPacketInternal> RawMediaPacket;

class RawMediaPacketInternal {

protected:
  std::vector<uint8_t> data;

public:
  RawMediaPacketInternal();
  RawMediaPacketInternal(std::vector<uint8_t>& _data, bool copy);
  ~RawMediaPacketInternal();

  void   setData(std::vector<uint8_t>& _data, bool copy);
  const std::vector<uint8_t>& getData();
  uint32 getSize();

};

static RawMediaPacket createRawMediaPacket()
{
  return std::make_shared<RawMediaPacketInternal>();
}

#endif /*RAWMEDIAPACKET_H_*/
