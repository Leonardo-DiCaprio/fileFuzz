/*
 * oggStreamEncoder is a class to insert an ogg packet into an ogg page stream
 *
 * Copyright (C) 2008-2009 Joern Seger
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

#ifndef OGGSTREAMENCODER_H_
#define OGGSTREAMENCODER_H_

#include <list>
#include <vector>

#include "mediaEncoder.h"
#include "oggPage.h"
#include "oggPacket.h"
#include "ringbuffer.h"
#include "definition.h"
#include <boost/circular_buffer.hpp>

class OggStreamEncoder : public MediaEncoder {

protected:
  static const uint32 maxSegmentEntries = 4096;

  static std::vector<uint32> m_usedSerialNo;

  uint32 m_maxPageSize;

  uint32 m_streamSerialNo;
  uint8  m_streamNo;

  std::list<OggPage> m_oggPageList;
  std::list<OggPacket> m_oggPacketList;
  boost::circular_buffer<uint8_t> m_segmentsBuffer;

  uint32 m_dataLength;   //!< is the length of the actually available data
  uint32 m_dataSegments; //!< is the number of the actually available segments
  uint32 m_usedData;     //!< is the size of data, that has already been used in the first packet

  uint32 m_pageCounter;

  uint32 findUniqueSerial(uint32 proposal);

  void   addPacket(OggPacket& packet);
  bool   getNextPacketLength(uint32 PageBorder, uint32& length, uint32& segments);
  void   createPage(uint32 minPageLength);

public:
  OggStreamEncoder(uint32 serial = 0);
  virtual ~OggStreamEncoder();

  virtual OggStreamEncoder& operator<<(OggPacket packet);
  virtual OggStreamEncoder& operator>>(OggPage& page);

  virtual void flush();

};

#endif /*OGGSTREAMENCODER_H_*/
