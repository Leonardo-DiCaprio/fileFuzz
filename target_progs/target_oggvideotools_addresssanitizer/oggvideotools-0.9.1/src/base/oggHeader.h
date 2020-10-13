/*
 * information about the ogg header
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

#ifndef oggHeader_h
#define oggHeader_h

#include "definition.h"

struct OggHeader {

  char    ogg[4];
  char    version;

#ifdef LITTLE_ENDIAN
  char    pack_type:1;
  char    page_type:1;
  char    last:1;
  char    reserved:5;
#else
  char    reserved:5;
  char    last:1;
  char    page_type:1;
  char    pack_type:1;
#endif
  int64   position;
  uint32  serial;
  uint32  pageNo;
  uint32  checksum;
  uint8   tableSegments;

} __attribute__ ((packed));

struct StreamType {
  unsigned char    headerType;
  char    typeName[6];
} __attribute__ ((packed));


#endif
