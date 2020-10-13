/*
 * simple ring buffer
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
#include "ringbuffer.h"
#include "exception.h"
#include "log.h"

ringbuffer::ringbuffer(uint32_t buffersize)
  : size(buffersize), used(0), begin(0), end(0)
{
  lock();
  logger.debug() <<"creating ringbuffer with size " << buffersize << std::endl;
  fifo.resize(buffersize); //reserve(buffersize);// = new unsigned char[buffersize];
  unlock();
}

//ringbuffer::ringbuffer(unsigned char* data, uint32_t len)
//    : size(len), used(len), begin(0), end(0)
//{
//  // gonna make a copy for safety:
//  lock();
//  fifo.reserve(len);// = new unsigned char[len];
//  fifo.insert(fifo.end(), data.data(), &data[len]); //memcpy(fifo, data, len);
//  unlock();
//}

ringbuffer::~ringbuffer()
{
}

//uint32_t ringbuffer::addData(const unsigned char* data, uint32_t len)
//{
//  lock();

//  if ((!len) || (!data)) {
//    unlock();
//    return(0);
//  }

//  if (len > size) {
//    unlock();
//    throw OggException("Ring buffer write overflow");
//  }
//  if (begin+len < size) {
//    memcpy(fifo.data()+begin,data,len);
//  } else {
//    // split
//    int part1 = (size - begin);
//    int part2 = len - part1;
//    memcpy(fifo.data()+begin,data,part1);
//    memcpy(fifo.data(),data+part1,part2);
//  }

//  begin += len;
//  begin %= size;

//  if (len > (size-used)) {
//    throw OggException("Ring buffer overrun");
//  } else {
//    used += len;
//  }

//  unlock();

//  return (len);

//}

uint32_t ringbuffer::addData(const std::vector<uint8_t>& data, uint32_t len)
{
  lock();

  if ((!len) || (data.empty())) {
    unlock();
    return(0);
  }

  if (len > size) {
    unlock();
    throw OggException("Ring buffer write overflow");
  }
  if (begin+len < size) {
    logger.debug() << "fifo: capacity " << fifo.capacity() << " size: "<< fifo.size() << std::endl;
    logger.debug() << "writing data with length " << len << " as one block " << std::endl;
    logger.debug() << "ringbuffer begin: " << begin << " end: " << end <<" size: " << size << std::endl;
    memcpy(fifo.data()+begin,data.data(),len);
  } else {
    // split
    int part1 = (size - begin);
    int part2 = len - part1;
    logger.debug() << "writing data with length " << len << " splitted " << part1 <<" / " << part2 << std::endl;
    logger.debug() << "ringbuffer begin: " << begin << " end: " << end <<" size: " << size << std::endl;
    memcpy(fifo.data()+begin,data.data(),part1);
    memcpy(fifo.data(),data.data()+part1,part2);
  }

  begin += len;
  begin %= size;

  if (len > (size-used)) {
    throw OggException("Ring buffer overrun");
  } else {
    used += len;
  }

  unlock();

  return (len);

}



uint32_t ringbuffer::getData(unsigned char* data, uint32_t len)
{
  lock();

  if (used < len)
    len = used;

  if (len > size) {
    throw OggException("Ring buffer read overflow");
  }
  if (size < (end + len)) {
    // split
    int part1 = size - end;
    int part2 = len - part1;
    memcpy(data, fifo.data()+end, part1);
    memcpy(data+part1, fifo.data(), part2);
  } else
    memcpy(data, fifo.data()+end, len);

  end += len;
  end %= size;
  used -= len;

  /*
  for(uint32_t i=0; i<len; ++i) {
    data[i] = fifo[end++];
    end %= size;
  }
  */

  unlock();

  return (len);
}

uint32_t ringbuffer::getData(std::vector<uint8_t>& data, uint32_t len)
{
  lock();

  if (used < len)
    len = used;

  if (len > size) {
    throw OggException("Ring buffer read overflow");
  }

  data.clear();
  data.reserve(len);
//  data.insert(data.end(),len);

  if (size < (end + len)) {
    // split
    int part1 = size - end;
    int part2 = len - (size-end);
    //data.insert(std::end(data),std::begin(fifo)+end, std::end(fifo)); //std::begin(fifo)+end+part1);
    data.insert(data.end(),fifo.begin()+end, fifo.end()); //std::end(fifo));
    data.insert(std::end(data),std::begin(fifo), std::begin(fifo)+part2);
//    memcpy(data.data(), fifo.data()+end, part1);
//    memcpy(data.data()+part1, fifo.data(), part2);
  } else {
    data.insert(std::end(data),std::begin(fifo)+end, std::begin(fifo)+end+len);
//    memcpy(data.data(), fifo.data()+end, len);
  }

  end += len;
  end %= size;
  used -= len;

  /*
  for(uint32_t i=0; i<len; ++i) {
    data[i] = fifo[end++];
    end %= size;
  }
  */

  unlock();

  return (len);
}

uint32_t ringbuffer::getAvailable()
{
  uint32_t retValue;

  lock();
  retValue = size-used;
  unlock();

  return(retValue);
}

uint32_t ringbuffer::getUsed()
{
  uint32_t retValue;

  lock();
  retValue = used;
  unlock();

  return(retValue);
}

void ringbuffer::clean()
{
  lock();
  begin = end = used = 0;
  unlock();
}

//uint32_t ringbuffer::peekBack(unsigned char* data, uint32_t len)
//{
//  lock();
//  if (len>used)
//    len = used;

//  int tmpEnd = begin-1;
//  for (int i=len-1; i>=0; --i) {
//    if (tmpEnd < 0)
//      tmpEnd = size-1;
//    data[i] = fifo[tmpEnd--];
//  }
//  unlock();

//  return (len);
//}

uint32_t ringbuffer::peekBack(std::vector<uint8_t>& data, uint32_t len)
{
  lock();
  if (len>used)
    len = used;

  /* clean data and fill with zeros in specified length */
  data.clear();
  data.insert(std::begin(data), len, 0);

  int tmpEnd = begin-1;
  for (int i=len-1; i>=0; --i) {
    if (tmpEnd < 0)
      tmpEnd = size-1;
    data[i] = fifo[tmpEnd--];
  }
  unlock();

  return (len);
}


//uint32_t ringbuffer::peekFront(unsigned char* data, uint32_t len)
//{
//  lock();
//  if (used < len)
//    len = used;

//  uint32_t tmpEnd = end;
//  for (uint32_t i=0; i<len; ++i) {
//    data[i] = fifo[tmpEnd++];
//    tmpEnd %= size;
//  }
//  unlock();

//  return (len);
//}

uint32_t ringbuffer::peekFront(std::vector<uint8_t>& data, uint32_t len)
{
  lock();
  if (used < len)
    len = used;

  /* clean data and fill with zeros in specified length */
  data.clear();
  data.insert(std::begin(data), len, 0);

  uint32_t tmpEnd = end;
  for (uint32_t i=0; i<len; ++i) {
    data[i] = fifo[tmpEnd++];
    tmpEnd %= size;
  }
  unlock();

  return (len);
}


uint32_t ringbuffer::inc(uint32_t len)
{
  lock();
  if (used < len)
    len = used;

  end += len;
  end %= size;
  used -= len;

  unlock();

  return (len);
}
