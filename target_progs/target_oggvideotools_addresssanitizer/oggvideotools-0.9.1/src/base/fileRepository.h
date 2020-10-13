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

#ifndef FILEREPOSITORY_H_
#define FILEREPOSITORY_H_

#include <stdio.h>

#include "definition.h"
#include "mediaRepository.h"
#include "rawMediaPacket.h"

class FileRepository : public MediaRepository {

  FILE*       fileDescriptor;
  std::string filename;
  uint32      bunchSize;
  bool        eof;

public:
  FileRepository();
  FileRepository(const std::string& filename, MediaDirection_t type);
  virtual ~FileRepository();

  virtual MediaUnit& operator<<(RawMediaPacket& packet);
  virtual MediaUnit& operator>>(RawMediaPacket& packet);

  virtual void setBunchSize(uint32 bunchSize);
  virtual uint32 getBunchSize();

  virtual void close();

  bool isEndOfFile();

};

#endif /*FILEREPOSITORY_H_*/
