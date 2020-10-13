/*
 * OggPage will carry all relevant information of an ogg page
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

#include <sstream>
#include <cstring>

#include "oggPage.h"
#include "oggHeader.h"
#include "crc.h"

/* OggPageInternal */

OggPageInternal::OggPageInternal()
  : m_dataPtr(0), m_headerLength(0), m_bodyLength(0), m_streamNo(255), m_empty(true)
{
}

OggPageInternal::OggPageInternal(std::vector<uint8_t>& _dataPtr, uint32 _headerLength, uint32 _bodyLength)
  : m_headerLength(_headerLength), m_bodyLength(_bodyLength),  m_streamNo(255), m_empty(false)
{
  m_dataPtr = _dataPtr;
}

OggPageInternal::~OggPageInternal()
{

}

bool OggPageInternal::isContinued()
{
  return(((OggHeader*)(&m_dataPtr[0]))->pack_type);
}

void OggPageInternal::setContinued()
{
  ((OggHeader*)(&m_dataPtr[0]))->pack_type = 1;
}

bool OggPageInternal::isBOS()
{
  return(((OggHeader*)(&m_dataPtr[0]))->page_type);
}

bool OggPageInternal::isEOS()
{
  return(((OggHeader*)(&m_dataPtr[0]))->last);
}

void OggPageInternal::setBOS()
{
  ((OggHeader*)(&m_dataPtr[0]))->page_type = 1;
}

void OggPageInternal::unsetBOS()
{
  ((OggHeader*)(&m_dataPtr[0]))->page_type = 0;
}

void OggPageInternal::setEOS()
{
  ((OggHeader*)(&m_dataPtr[0]))->last = 1;
}

void OggPageInternal::unsetEOS()
{
  ((OggHeader*)(&m_dataPtr[0]))->last = 0;
}

void OggPageInternal::setStreamNo(uint8 streamNo)
{
  m_streamNo = streamNo;
}

uint8 OggPageInternal::getStreamNo()
{
  return(m_streamNo);
}

uint32 OggPageInternal::version()
{
  return(((OggHeader*)(&m_dataPtr[0]))->version);
}

uint32 OggPageInternal::packets()
{
  uint32 segments(((OggHeader*)(&m_dataPtr[0]))->tableSegments);
  uint32 packets(0);
  uint8* oggPtr=&m_dataPtr[0]+sizeof(OggHeader);

  for (uint32 i(0); i<segments; ++i)
    if (oggPtr[i]<0xff)packets++;

  return(packets);

}

int64 OggPageInternal::granulepos()
{
  return(((OggHeader*)(&m_dataPtr[0]))->position);
}

uint32 OggPageInternal::serialno()
{
  return(((OggHeader*)(&m_dataPtr[0]))->serial);
}

uint32 OggPageInternal::pageno()
{
  return(((OggHeader*)(&m_dataPtr[0]))->pageNo);
}

uint32 OggPageInternal::length()
{
  return(m_headerLength + m_bodyLength);
}

std::vector<uint8_t>& OggPageInternal::data()
{
  return(m_dataPtr);
}

bool OggPageInternal::isEmpty()
{
  return(m_empty);
}

void OggPageInternal::createCRC()
{
  OggHeader* hdr = (OggHeader*)(&m_dataPtr[0]);
  hdr->checksum  = 0;
  hdr->checksum  = Crc::create(&m_dataPtr[0], length());
}

OggPage OggPageInternal::clone()
{
  OggPage page;

  if (!m_dataPtr.empty()) {

    page = std::make_shared<OggPageInternal>(m_dataPtr, m_headerLength, m_bodyLength);

  }

  return page;
}

OggPage OggPageInternal::create(std::vector<uint8_t>& data, uint32_t headerLength, uint32_t bodyLength)
{
  OggPage page;

  if (length() > 0) {
    page = std::make_shared<OggPageInternal>(data, headerLength, bodyLength);
  }

  return page;
}

OggPage OggPageInternal::getPtr()
{
  return shared_from_this();
}

/* toString levels:
 * 0) only data length information
 * 1) header information
 * 2) additional header information
 * 3) header dump
 * 4) body dump
 */
std::string OggPageInternal::toString(uint8 level)
{
  std::stringstream retStream;

  retStream << "Ogg Page: header length = " << std::dec << m_headerLength
            << " and body length = " << std::dec << m_bodyLength
            << std::endl;

  if (level < 1)
    return(retStream.str());

  OggHeader* header = (OggHeader*)(&m_dataPtr[0]);
  retStream << "Header Information:"
            << "\n\tOgg Version      : " << (uint32)header->version
            << "\n\tSerial No        : 0x" << std::hex << header->serial << std::dec
            << "\n\tPacket Type      : ";

  if (header->pack_type)
    retStream << "continued packet";
  else
    retStream << "fresh packet";

  retStream << "\n\tPage Type        : ";

  if (header->page_type)
    retStream << "begin of stream marker";
  else
    retStream << "normal page";

  retStream << "\n\tLast Page        : ";

  if (header->last)
    retStream << "end of stream marker";
  else
    retStream << "normal page";

  retStream << "\n\tGranule Position : " << header->position << "(0x" << std::hex << header->position << std::dec << ")";
  retStream << "\n\tPage Number      : " << header->pageNo;
  retStream << "\n\tChecksum         : 0x" << std::hex << header->checksum << std::dec;
  retStream << "\n\tTable Segments   : " << (uint32) header->tableSegments;
  retStream << std::endl << std::endl;

  if (level < 2)
    return(retStream.str());

  retStream << "Segments:";

  for (uint32 c(0); c<header->tableSegments; ++c) {
    if ((c%16) == 0)
      retStream << std::endl;
    retStream << " "<< std::hex;
    if (((unsigned int) (m_dataPtr[c+sizeof(OggHeader)])) < 16)
      retStream << "0";
    retStream << (unsigned int) (m_dataPtr[c+sizeof(OggHeader)]);
  }

  retStream << std::endl << std::endl;

  if (level < 3)
    return(retStream.str());

  retStream << "Header Hex dump: ";
  for (uint32 c(0); c<m_headerLength; ++c) {
    if ((c%16) == 0)
      retStream << std::endl;
    retStream << " " << std::hex;
    if (((unsigned int) (m_dataPtr[c])) < 16)
      retStream << "0";
    retStream << (unsigned int) (m_dataPtr[c]);
  }
  retStream << std::dec << std::endl << std::endl;

  if (level < 4)
    return(retStream.str());

  retStream << "Body Hex dump: ";

  for (uint32 c(0); c<m_bodyLength; ++c) {
    if ((c%32) == 0)
      retStream << std::endl;
    retStream << " " << std::hex;
    if (((unsigned int) (m_dataPtr[c+m_headerLength])) < 16)
      retStream << "0";
    retStream << (unsigned int) (m_dataPtr[c+m_headerLength]);
  }

  retStream << std::dec << std::endl;

  return(retStream.str());
}

