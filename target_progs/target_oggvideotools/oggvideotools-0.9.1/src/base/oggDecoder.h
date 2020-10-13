/*
 * OggDecoder
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

#ifndef OGGDECODER_H_
#define OGGDECODER_H_

#include <list>
#include <vector>

#include "mediaDecoder.h"
#include "rawMediaPacket.h"
#include "oggPage.h"
#include "oggRingbuffer.h"
#include <queue>
#include <boost/circular_buffer.hpp>

//! class to decode a raw bytestream into Ogg packets
/*! This class awaits raw packets (in form of a RawMediaPacket). These
 *  raw packets were inserted into a ringbuffer, from which the Ogg packets
 *  are extracted.
 *
 *  When a raw packet has been inserted into the ring buffer, the OggDecoder
 *  tries to extract one or more Ogg packets immediately. Therefor in normal
 *  operations, the ring buffer can not overflow, if the ring buffer size is
 *  big enough to hold a full Ogg Packet.
 *
 *
 *  Example:
  \code
RawMediaPacket rawPacket;
OggPage        oggPage;

FileRepository m_repository("myfile.ogg", MediaUnit::read);
OggDecoder     oggDecoder;

m_repository >> rawPacket;
oggDecoder << rawPacket;

if (oggDecoder.isAvailable())
  oggDecoder >> oggPage;
\endcode
 * */
class OggDecoder : public MediaDecoder {

protected:
  boost::circular_buffer<uint8_t>  m_oggRingbuffer;
  std::queue<OggPage>              m_oggPageList;

  void getNextPages();

public:
  OggDecoder();
  virtual ~OggDecoder();

  //! Stream input method
  /*! This method inserts a RawMediaPacket, which is just a bunch of
   *  raw bytes, into the decoder. When a new packet arrives, the
   *  decoder tries to extract the oggPages, which are fully available.
   *  The oggPages will then be buffered within a m_oggPageList.
   *  The uninterpreted data is stored until the next rawdata is received.
   *  @param mediaPacket This is the raw media packet, created by a m_repository.
   *  @return A reference to the actual OggDecoder object.
   * */
  virtual OggDecoder& operator<<(RawMediaPacket& mediaPacket);

  //! Stream output method
  /*! This method returns the next oggPage within the list of OggPages.
   *  If there is no page availabe, this method leaves the OggPage object untoched.
   *  @param oggPage the oggPage to be filled.
   *  @return A reference to the actual OggDecoder object.
   * */
  virtual OggDecoder& operator>>(OggPage& page);

  uint32_t getNextPageLength();

  virtual void clear();

  uint32 space();
};

#endif /*OGGDECODER_H_*/
