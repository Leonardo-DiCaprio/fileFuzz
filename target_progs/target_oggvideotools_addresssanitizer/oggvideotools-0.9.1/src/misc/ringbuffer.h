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

#ifndef ringbuffer_h
#define ringbuffer_h

#include <cstdint>
#include <vector>

class ringbuffer {

protected:
  //unsigned char* fifo;
  std::vector<std::uint8_t> fifo;

  uint32_t size;
  uint32_t used;
  uint32_t begin; //! first available sign
  uint32_t end;   //! oldest packet

  void lock() {}
  void unlock() {}

public:
  ringbuffer(uint32_t buffersize = 8000);
//  ringbuffer(uint8_t* rawdata, uint32_t len);

  ~ringbuffer();

//  uint32_t addData(const unsigned char* data, uint32_t len);
  uint32_t getData(unsigned char* data, uint32_t len);

  uint32_t getAvailable();
  uint32_t getUsed();

  // read newest nBytes
//  uint32_t peekBack(unsigned char* data, uint32_t len);

  // read oldest nBytes
//  uint32_t peekFront(unsigned char* data, uint32_t len);

  // delete the oldest len bytes
  uint32_t inc(uint32_t len);

  void clean();

  uint32_t addData(const std::vector<uint8_t> &data, uint32_t len);
  uint32_t getData(std::vector<uint8_t> &data, uint32_t len);
  uint32_t peekBack(std::vector<uint8_t> &data, uint32_t len);
  uint32_t peekFront(std::vector<uint8_t> &data, uint32_t len);
};

#endif
