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

#ifndef OGGPACKET_H_
#define OGGPACKET_H_

#include <string>
#include <memory>
#include <vector>

#ifdef HAVE_LIBOGG
#include <ogg/ogg.h>
#endif

#include "definition.h"
//#include "refObject.h"
#include "oggTypes.h"

class OggPacketInternal;

typedef std::shared_ptr<OggPacketInternal> OggPacket;

class OggPacketInternal : public std::enable_shared_from_this<OggPacketInternal> {

public:

  enum class PacketType {
    normal,
    bos,
    eos
  };

protected:
  ogg_packet m_oggPacket;

  /* information about the stream type and the stream No */
  OggType m_streamType;
  uint8   m_streamNo;
  bool    m_streamHeader;

public:
  OggPacketInternal();
  OggPacketInternal(const ogg_packet& ogg_p);
  OggPacketInternal(uint8* data, uint32 length, uint32 packetNo,
                    int64 granulePos=-1, PacketType packetType = PacketType::normal);
  OggPacketInternal(std::vector<uint8> data, uint32 packetNo, int64 granulePos, PacketType packetType);

  virtual ~OggPacketInternal();

  OggPacket clone();
  OggPacket getPtr();
  static OggPacket create(uint8* data, uint32 length, uint32 packetNo,
                          int64 granulePos=-1, PacketType packetType = PacketType::normal);

  int64   granulepos();

  void    setGranulepos(int64 pos);

  uint32  getPacketNo();
  uint8   getStreamNo();
  OggType getStreamType();
  ogg_packet* getUnderlayingOggPacketPtr() {
    return &m_oggPacket;
  }
//  void    cleanPacketPtr() { m_oggPacket.packet = 0; /* no delete */ }

  void    setStreamType(OggType type);
  void    setStreamNo(uint8 streamNo);
  void    setStreamHeader();

  bool    isBOS();
  bool    isEOS();
  bool    isStreamHeader();

  void    setBOS();
  void    unsetBOS();
  void    setEOS();
  void    unsetEOS();

  void    setPacketno(int64_t no) {
    m_oggPacket.packetno = no;
  }

  void liboggDelivery();

  uint32 length();
  uint8* data();

  /*
    ogg_packet toLibogg();
    void fromLibogg(ogg_packet packet);
  */

  std::string toString(uint8 level);

};

#endif /*OGGPACKET_H_*/
