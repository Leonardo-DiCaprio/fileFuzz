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

#include <iostream>
#include <string.h>
#include <stdlib.h>

#include "oggDecoder.h"
#include "oggHeader.h"
#include "log.h"
#include "exception.h"

#define min(a,b) (((a)<(b))?(a):(b));

OggDecoder::OggDecoder()
  : m_oggRingbuffer(71000) // max page size plus 4KB read size
{
  setConfigured();
}

OggDecoder::~OggDecoder()
{
}

void OggDecoder::clear()
{
  /* nothing to be done here */
}

void OggDecoder::getNextPages()
{

  while (1) {
    uint32 length(0);

    if (length = getNextPageLength()) {

      logger.debug() << "getNextPage: length="<<length<< " available length in ringbuffer=" << m_oggRingbuffer.size()
                     <<  std::endl;

      std::vector<uint8_t> data;//(length,0);
      data.insert(data.end(), m_oggRingbuffer.begin(), m_oggRingbuffer.begin()+length);
      m_oggRingbuffer.erase_begin(length); //m_oggRingbuffer.begin(), m_oggRingbuffer.begin()+length);
      uint32 headerLength = sizeof(OggHeader) + ((OggHeader*)data.data())->tableSegments;
      uint32 bodyLength = length - headerLength;

      auto page = std::make_shared<OggPageInternal>(data, headerLength, bodyLength);

      m_oggPageList.push(page);

      setAvailable();
    } else
      break;
  }
}

OggDecoder& OggDecoder::operator<<(RawMediaPacket& mediaPacket)
{
  logger.debug() << "inserting: ringbuffersize before: " << m_oggRingbuffer.size() << " insert length="<<mediaPacket->getSize() << std::endl;
  /* insert the raw data into the ring buffer*/
  m_oggRingbuffer.insert(m_oggRingbuffer.end(), mediaPacket->getData().begin(), mediaPacket->getData().end());

  logger.debug() << "inserting: ringbuffersize after: " << m_oggRingbuffer.size() << std::endl;

  /* extract ogg pages */
  getNextPages();

  return(*this);
}

OggDecoder& OggDecoder::operator>>(OggPage& page)
{
  if (isAvailable()) {
    page = m_oggPageList.front();
    m_oggPageList.pop();
    if (m_oggPageList.empty())
      setEmpty();
  } else
    logger.error() << "OggDecoder::operator>>: no page available, insert a packet first\n";

  return(*this);
}

uint32 OggDecoder::space()
{
  return (uint32_t) (m_oggRingbuffer.capacity()-m_oggRingbuffer.size());
}

uint32_t OggDecoder::getNextPageLength()
{

  uint32_t availLength = (uint32_t) m_oggRingbuffer.size();
  logger.debug() << "get new page length (if one) " << std::endl;

  if (availLength < (int) sizeof(OggHeader)) {
    return (0);
  }

  // test is this aligned?
  char starter[5];
  uint32_t pos;
  for (pos = 0; pos < 5; ++pos) {
    starter[pos] = m_oggRingbuffer[pos];
    logger.debug() << "data " << std::hex << "0x" << (int) starter[pos] << " (" << std::dec << starter[pos] << ")\n";
  }

  if (strncmp(starter, "OggS", 4) != 0) {
    logger.debug() << "Error: ogg string is " << std::hex << "0x" << (int) starter[0]
                   << " 0x" << (int) starter[1] << " 0x" << (int) starter[2]
                   << " 0x" << (int) starter[3] << " 0x" << (int) starter[4] << std::endl;
    throw OggException("OggRingbuffer::getNextPageLength: ERROR ogg packet not aligned");
  }

  if ('\0' != starter[4]) {
    throw OggException("OggRingbuffer::getNextPageLength: ERROR unsupported stream structure version");
  }

  //pos = sizeof(OggHeader) - 1; // jump to the segment table

  uint32_t readsegments = m_oggRingbuffer[26];

  logger.debug() << "there are " << readsegments << " segments available in this packet";

  if (availLength < (int) (sizeof(OggHeader) + readsegments)) {
    return (0);
  }

  // what is the data length
  uint32_t data_length(0);
  uint32_t header_offset(sizeof(OggHeader));

  for (uint32_t i = 0; i < readsegments; ++i) {
    data_length += m_oggRingbuffer[header_offset + i];
  }

  uint32_t overallLength = data_length + header_offset  + readsegments;
  return (overallLength<=availLength?overallLength:0);
}