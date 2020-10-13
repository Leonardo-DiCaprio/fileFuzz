/*
 * MediaUnit is a baseclass for all media transfer units
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

#ifndef MEDIAUNIT_H_
#define MEDIAUNIT_H_

#include <string>

//#include "MediaConfig.h"

//! Base class for the media processing units
/*! This abstract class is the base class for all following classes, that process media data
 *  A MediaUnit object is able to receive packets and to send
 *  following classes, that process media data
 *  The idea is to connect one mediaUnit Object with
 *  another by using connectFront and connectBack.
 */

class MediaUnit {

public:
  enum MediaDirection_t {
    write,
    read,
    readwrite
  };

protected:
  std::string      name;
  MediaDirection_t mediaDirection;

public:
  MediaUnit(MediaDirection_t type, const std::string name);
  virtual ~MediaUnit();

};

#endif /*MEDIAUNIT_H_*/
