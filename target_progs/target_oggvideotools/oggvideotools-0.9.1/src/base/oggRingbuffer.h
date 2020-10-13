/*
 * Ringbuffer to prebuffer an ogg file
 *
 * Copyright (C) 2005-2009 Joern Seger
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
#ifndef oggRingbuffer_h
#define oggRingbuffer_h

#include "ringbuffer.h"
#include "oggHeader.h"

class OggRingbuffer : public ringbuffer {

protected:
  void dump();

public:
  OggRingbuffer(uint32_t buffersize = 64000);
  OggRingbuffer(uint8_t* data, uint32_t len);
  ~OggRingbuffer();

  bool getNextPageLength(uint32_t& length, int pageNum=1);
  bool getNextPage(uint8_t*& data, uint32_t& length);
  bool getNextPages(uint8_t*& data, uint32_t& length, uint32_t pageNum);

  bool getNextPages(std::vector<uint8_t> &data, uint32_t &length, uint32_t size);
  bool getNextPage(std::vector<uint8_t> &data, uint32_t &length);
};


#endif
