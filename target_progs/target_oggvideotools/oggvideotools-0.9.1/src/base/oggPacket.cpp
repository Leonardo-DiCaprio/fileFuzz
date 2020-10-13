/*
 * OggPacket will carry all relevant information of an ogg packet
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

#include <iostream>
#include <sstream>
#include <cstring>
#include <ogg/ogg.h>

#include "oggPacket.h"

OggPacketInternal::OggPacketInternal()
  : m_streamType(OggType::unknown), m_streamNo(255), m_streamHeader(false)
{
  // create a packet without any extra data
  memset(&m_oggPacket,0, sizeof(m_oggPacket));
  m_oggPacket.granulepos = -1;

}

OggPacketInternal::OggPacketInternal(const ogg_packet& pkt)
  : m_streamType(OggType::unknown), m_streamNo(255), m_streamHeader(false)
{

  m_oggPacket = pkt;

  // it is unclear, who is the owner of the data!?
  uint8_t* tmp = new uint8_t [pkt.bytes];
  memcpy(tmp, pkt.packet, pkt.bytes);
  m_oggPacket.packet = tmp;

}

OggPacketInternal::OggPacketInternal(uint8 *data, uint32 length,
                                     uint32 packetNo, int64 granulePos, PacketType packetType)
  : m_streamType(OggType::unknown), m_streamNo(255), m_streamHeader(false)
{
  uint8_t* tmp_data(0);

  tmp_data = new uint8_t[length];
  memcpy(tmp_data, data, length);

  m_oggPacket.packet     = tmp_data;
  m_oggPacket.bytes      = length;
  m_oggPacket.b_o_s      = 0;
  m_oggPacket.e_o_s      = 0;
  m_oggPacket.granulepos = granulePos;
  m_oggPacket.packetno   = packetNo;

  switch (packetType) {
  case PacketType::bos:
    m_oggPacket.b_o_s = 256;
    break;
  case PacketType::eos:
    m_oggPacket.e_o_s = 256;
    break;
  default: {
  }
  }

}

OggPacketInternal::OggPacketInternal(std::vector<uint8> data, uint32 packetNo, int64 granulePos, PacketType packetType)
  : m_streamType(OggType::unknown), m_streamNo(255), m_streamHeader(false)
{
  uint8_t* tmp_data(0);

  if (data.size() > 0) {
    tmp_data = new uint8_t[data.size()];
    memcpy(tmp_data, &data[0], data.size());
  }

  m_oggPacket.packet     = tmp_data;
  m_oggPacket.bytes      = data.size();
  m_oggPacket.b_o_s      = 0;
  m_oggPacket.e_o_s      = 0;
  m_oggPacket.granulepos = granulePos;
  m_oggPacket.packetno   = packetNo;

  switch (packetType) {
  case PacketType::bos:
    m_oggPacket.b_o_s = 256;
    break;
  case PacketType::eos:
    m_oggPacket.e_o_s = 256;
    break;
  default: {
  }
  }

}



OggPacket OggPacketInternal::clone()
{
  OggPacket pkt = std::make_shared<OggPacketInternal>(m_oggPacket);

  /* a bit nasty, as ogg_packet is c */
//  uint8* data = new uint8[m_oggPacket.bytes];
//  memcpy(data, m_oggPacket.packet, m_oggPacket.bytes);
//
//  PacketType packetType(PacketType::normal);
//
//  if (m_oggPacket.b_o_s)
//    packetType = PacketType::bos;
//
//  if (m_oggPacket.e_o_s)
//    packetType = PacketType::eos;
//
//  OggPacket pkt = std::make_shared<OggPacketInternal>(data, m_oggPacket.bytes, m_oggPacket.packetno, m_oggPacket.granulepos, packetType);
//
//  pkt->m_streamNo = m_streamNo;
//  pkt->m_streamType = m_streamType;
//  pkt->m_streamHeader = m_streamHeader;

  return pkt;
}

void OggPacketInternal::liboggDelivery()
{
  uint8_t* tmp(new uint8_t [m_oggPacket.bytes]);
  memcpy(tmp, m_oggPacket.packet, m_oggPacket.bytes);
  m_oggPacket.packet = tmp;

  // libtheora/libvorbis or whatever - do whatever is neccessary with your memory
  // I have my copy to use it wherever I like ;-)

}

OggPacket OggPacketInternal::getPtr()
{
  return shared_from_this();
}

OggPacket OggPacketInternal::create(uint8 *data, uint32 length, uint32 packetNo, int64 granulePos, OggPacketInternal::PacketType packetType)
{
  return std::make_shared<OggPacketInternal>(data, length, packetNo, granulePos, packetType);
}


OggPacketInternal::~OggPacketInternal()
{
  if (!m_streamHeader)
    delete[] m_oggPacket.packet;
}


int64 OggPacketInternal::granulepos()
{
  return m_oggPacket.granulepos;
}

void OggPacketInternal::setGranulepos(int64 pos)
{
  m_oggPacket.granulepos = pos;
}

void OggPacketInternal::setStreamHeader()
{
  m_streamHeader = true;
}

bool OggPacketInternal::isStreamHeader()
{
  return m_streamHeader;
}

uint32 OggPacketInternal::getPacketNo()
{
  return m_oggPacket.packetno;
}

uint32 OggPacketInternal::length()
{
  return (uint32) m_oggPacket.bytes;
}

bool OggPacketInternal::isBOS()
{
  return m_oggPacket.b_o_s != 0;
}

bool OggPacketInternal::isEOS()
{
  return m_oggPacket.e_o_s != 0;
}

void OggPacketInternal::setBOS()
{
  m_oggPacket.b_o_s = 1;
}

void OggPacketInternal::setEOS()
{
  m_oggPacket.e_o_s = 1;
}

void OggPacketInternal::unsetBOS()
{
  m_oggPacket.b_o_s = 0;
}

void OggPacketInternal::unsetEOS()
{
  m_oggPacket.e_o_s = 0;
}

/*
ogg_packet OggPacketInternal::toLibogg()
{
  return(*objPtr);
}
*/
uint8 OggPacketInternal::getStreamNo()
{
  return m_streamNo;
}

OggType OggPacketInternal::getStreamType()
{
  return m_streamType;
}

void OggPacketInternal::setStreamNo(uint8 no)
{
  m_streamNo = no;
}

void OggPacketInternal::setStreamType(OggType type)
{
  m_streamType = type;
}

uint8* OggPacketInternal::data()
{
  return m_oggPacket.packet;
}

/* toString levels:
 * 1) only data length information
 * 2) header information
 * 3) additional header information
 * 4) header dump
 * 5) body dump
 */
std::string OggPacketInternal::toString(uint8 level)
{
  std::stringstream retStream;

  retStream << "\nOgg Packet: packet length = " << m_oggPacket.bytes << std::endl;

  if (level < 1)
    return(retStream.str());

  retStream << "\nHeader Information:"
            << "\n\tBegin of Stream     : ";

  if (m_oggPacket.b_o_s)
    retStream << "true";
  else
    retStream << "false";

  retStream << "\n\tEnd of Stream       : ";

  if (m_oggPacket.e_o_s)
    retStream << "true";
  else
    retStream << "false";

  retStream << "\n\tGranule Position    : " << m_oggPacket.granulepos;
  retStream << "\n\tPacket Number       : " << m_oggPacket.packetno;

  retStream << std::endl;

  if (level < 3)
    return(retStream.str());

  retStream << "\n\tStream Number       : " << (int)m_streamNo;
  retStream << "\n\tStream Type         : ";

  switch (m_streamType) {
  case OggType::vorbis:
    retStream << "Vorbis";
    break;
  case OggType::theora:
    retStream << "Theora";
    break;
  case OggType::kate:
    retStream << "Kate";
    break;
  case OggType::unknown:
  default:
    retStream << "unknown";
    break;
  }
  retStream << std::endl;

  if (level < 4)
    return(retStream.str());

  retStream << "\nPacket Hex dump:" << std::hex;

  for (int32 c(0); c<m_oggPacket.bytes; ++c) {
    if ((c%32) == 0)
      retStream << std::endl;
    if (((unsigned) (m_oggPacket.packet[c])) < 16)
      retStream << " 0";
    else
      retStream << " ";

    retStream << (unsigned) (m_oggPacket.packet[c]);
  }

  retStream << std::dec << std::endl;

  return(retStream.str());
}
