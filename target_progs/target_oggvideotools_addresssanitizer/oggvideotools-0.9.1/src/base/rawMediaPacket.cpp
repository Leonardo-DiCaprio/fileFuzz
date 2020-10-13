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

#include <string.h>
#include "rawMediaPacket.h"

RawMediaPacketInternal::RawMediaPacketInternal()
{
}

RawMediaPacketInternal::RawMediaPacketInternal(std::vector<uint8_t>& _data, bool copy)
{
  setData(_data, copy);
}

RawMediaPacketInternal::~RawMediaPacketInternal()
{
}

const std::vector<uint8_t> &RawMediaPacketInternal::getData()
{
  return data;
}

void RawMediaPacketInternal::setData(std::vector<uint8_t>& _data, bool copy)
{
//    data = _data;

  if (copy) {
    data = _data;
  } else {
    data = std::move(_data);
  }
}

uint32 RawMediaPacketInternal::getSize()
{
  return(data.size());
}

/*********************************/

//RawMediaPacket::RawMediaPacket()
//{
//}

//RawMediaPacket::RawMediaPacket(const RawMediaPacket& packet)
//    : RefObject<RawMediaPacketInternal>(packet)
//{
//}

//RawMediaPacket::RawMediaPacket(RawMediaPacketInternal* m_dataPtr)
//    : RefObject<RawMediaPacketInternal>(m_dataPtr)
//{
//}

//RawMediaPacket::~RawMediaPacket()
//{
//}

//uint8* RawMediaPacket::getData(uint32& length)
//{
//  return(objPtr->getData(length));
//}

//uint8* RawMediaPacket::getData()
//{
//  return(objPtr->getData());
//}

//uint32 RawMediaPacket::size()
//{
//  return(objPtr->size());
//}

