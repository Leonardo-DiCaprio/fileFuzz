/*
 * Ringbuffer to prebuffer an ogg file
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

/* History:
    01 2008: initial version is taken from the streamnik server project (JS)
*/

#include <iostream>
#include <string.h>

#include <stdlib.h>

#include "oggRingbuffer.h"
#include "oggHeader.h"
#include "exception.h"
#include "oggPage.h"
#include "log.h"

OggRingbuffer::OggRingbuffer(uint32_t buffersize)
  :ringbuffer(buffersize)
{
}

//OggRingbuffer::OggRingbuffer(uint8_t* data, uint32_t len)
//    :ringbuffer(data, len)
//{
//}


OggRingbuffer::~OggRingbuffer()
{
}

bool OggRingbuffer::getNextPageLength(uint32_t& length, int pageNum)
{
  lock();

  int tmpend  = end;
  int tmpused = used;
  length = 0;

  for (; pageNum; pageNum--) {

    logger.debug() << "get new page no " << pageNum << " available data is "<< tmpused << std::endl;
    uint32_t tmplen = 0;

    if (tmpused < (int) sizeof(OggHeader)) {
      unlock();
      return(false);
    }

    // test is this aligned?
    char starter[5];
    for (uint32_t i=0; i<5; ++i) {
      starter[i] = fifo[(tmpend+i)%size];
      logger.debug() << "data " << std::hex << "0x" << (int)starter[i] << " ("<<std::dec <<starter[i]<<")\n";
    }
    tmpend+=5;
    tmpend%=size;

    if (strncmp(starter, "OggS", 4) != 0) {
      unlock();
      logger.debug() << "Error: ogg string is " << std::hex << "0x" << (int)starter[0]
                     << " 0x" << (int)starter[1] << " 0x" << (int)starter[2]
                     << " 0x" << (int)starter[3] << " 0x" << (int)starter[4] << std::endl;
      dump();
      throw OggException("OggRingbuffer::getNextPageLength: ERROR ogg packet not aligned");
    }

    if ('\0' != starter[4]) {
      unlock();
      throw OggException("OggRingbuffer::getNextPageLength: ERROR unsupported stream structure version");
    }

    tmpend += sizeof(OggHeader)-6; // jump to the segment table
    tmpend %= size;

    uint32_t readsegments = fifo[tmpend];

    tmpend += 1;
    tmpend %= size;

    tmplen += sizeof(OggHeader) + readsegments;

    if (tmpused < (int)(sizeof(OggHeader)+readsegments)) {
      unlock();
      return(false);
    }

    for (uint32_t i=0; i<readsegments; ++i) {
      tmplen += fifo[tmpend];
      tmpend += 1;
      tmpend %= size;
    }

    if (tmpused < (int)tmplen) {
      unlock();
      return(false);
    }

    length += tmplen;
    tmpused -= tmplen;
    tmpend = end + length;
    tmpend %= size;

  }
  unlock();
  return(true);

}

//bool OggRingbuffer::getNextPages(uint8_t*& data, uint32_t& length, uint32_t size)
//{
//  if (!used)
//    return(false);

//  if (!getNextPageLength(length,size))
//    return(false);

//  if (!data)
//    data = new uint8_t[length];

//  if (length != getData(data, length))
//    return(false);


//  return(true);
//}

bool OggRingbuffer::getNextPages(std::vector<uint8_t>& data, uint32_t& length, uint32_t size)
{
  if (!used)
    return(false);

  if (!getNextPageLength(length,size))
    return(false);

  if (length != getData(data, length))
    return(false);


  return(true);
}



//bool OggRingbuffer::getNextPage(uint8_t*& data, uint32_t& length)
//{
//  return(getNextPages(data, length, 1));
//}

bool OggRingbuffer::getNextPage(std::vector<uint8_t>& data, uint32_t& length)
{
  return(getNextPages(data, length, 1));
}

void OggRingbuffer::dump()
{

  for (uint32 c(0); c<used; ++c) {
    if ((c%16) == 0)
      std::cerr << std::endl;
    std::cerr << " " << std::hex;
    if (fifo[(c+end)%size] < 16)
      std::cerr << "0";
    std::cerr << (uint32_t)fifo[(c+end)%size];
  }

  std::cerr << std::dec << std::endl;
}

