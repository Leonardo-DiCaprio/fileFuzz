/*
 * MediaRepository is a baseclass for all communication interfaces
 * (e.g. files, tcp, rtp and http streams)
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

#ifndef MEDIAREPOSITORY_H_
#define MEDIAREPOSITORY_H_

#include "definition.h"
#include "mediaUnit.h"
#include "rawMediaPacket.h"

//! Baseclass for all communication interfaces.
/*! This class is used as a basis for subsequent derived classes that are able to
 *  handle raw data from any source.
 *
 *  Actually the only implementation is the FileRepository class. One of
 *  the next implementations will be a http m_repository, that creates raw packets
 *  from an http media source or throughs out an http media stream.
 *
 * */
class MediaRepository : public MediaUnit {

protected:
  bool repositoryAvailable;

public:
  MediaRepository();

  //! Constructor to create a new MediaRepository object
  /*! @param type The media direction
   *   - MediaUnit::read for opening the m_repository for reading
   *   - MediaUnit::write for opening the m_repository for writing
   *  @param name The name of the media m_repository, this could be a filename, a url etc.
   *
   * */
  MediaRepository(MediaDirection_t type,const std::string name);

  virtual ~MediaRepository();

  //! Find out, if data is available
  /*! @return true, if any data is available, false if not */
  virtual bool isAvailable();

  //! Insert a new raw data packet into the m_repository
  virtual MediaUnit& operator<<(RawMediaPacket& packet) = 0;

  //! Receive a new raw data packet from the m_repository
  virtual MediaUnit& operator>>(RawMediaPacket& packet) = 0;

  //! Request the actual size of a raw media packet that this m_repository creates
  /*! @return Returns the bunch size, that is actually used.
   * */
  virtual uint32 getBunchSize() = 0;

  //! Configures the size of the raw media packet size
  /*! @param size The size to which all packets should be filled */
  virtual void setBunchSize(uint32 size) = 0;

  virtual void close() = 0;

};

#endif /*MEDIAREPOSITORY_H_*/
