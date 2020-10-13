/*
 * FileRepository is used to read or write a local file
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
#include <cstring>
#include <cerrno>
// #include <fstream> .. later

#include "rawMediaPacket.h"
#include "fileRepository.h"
#include "exception.h"
#include "log.h"

FileRepository::FileRepository()
  : MediaRepository(read, "FileRepository"), fileDescriptor(0), filename(""), bunchSize(4096),
    eof(true)
{

}

FileRepository::FileRepository(const std::string& _filename, MediaDirection_t type)
  : MediaRepository(type, "FileRepository"), filename(_filename), bunchSize(4096), eof(false)
{
  eof = true;
  if (mediaDirection == write) {
    if ((fileDescriptor = fopen(filename.c_str(), "wb")) == 0)
      logger.error() << name << "::open failed: " << strerror(errno) << "\n";
    else {
      repositoryAvailable = true;
      eof = false;
    }
  } else {
    if ((fileDescriptor = fopen(filename.c_str(), "rb")) == 0)
      logger.error() << name << "::open failed: "  << strerror(errno) << "\n";
    else {
      repositoryAvailable = true;
      eof = false;
    }
  }
}

FileRepository::~FileRepository()
{
}

void FileRepository::close()
{
  if (fileDescriptor) {
    fclose(fileDescriptor);
    fileDescriptor = 0;
    repositoryAvailable = false;
  }
}

void FileRepository::setBunchSize(uint32 _bunchSize)
{
  bunchSize = _bunchSize;
}

uint32 FileRepository::getBunchSize()
{
  return(bunchSize);
}

bool FileRepository::isEndOfFile()
{
  return(eof);
}

MediaUnit& FileRepository::operator<<(RawMediaPacket& packet)
{
  if ((mediaDirection == write) && (repositoryAvailable == true)) {
    if (fwrite(&(packet->getData())[0],1,packet->getSize(),fileDescriptor) != packet->getSize()) {
      throw OggException(name+"::operator<<: "+strerror(errno));
    }
  }

  return(*this);
}

MediaUnit& FileRepository::operator>>(RawMediaPacket& packet)
{
  uint32 readBytes;
  std::vector<uint8_t> buffer(bunchSize);

  if (mediaDirection == read) {

    if ((readBytes = fread(buffer.data(),1,bunchSize,fileDescriptor)) < 0) {
      logger.error() << name << "::operator>>: "<< strerror(errno) << "\n";
      return(*this);
    }

    if (buffer.size() > readBytes)
      buffer.resize(readBytes);

    packet = std::make_shared<RawMediaPacketInternal>(buffer, false); // do not create copy

    if (readBytes < bunchSize) {
      repositoryAvailable = false;
      eof = true;
    }
  }

  return (*this);
}
