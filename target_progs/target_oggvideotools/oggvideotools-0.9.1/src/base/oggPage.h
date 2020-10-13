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

#ifndef OGGPAGE_H_
#define OGGPAGE_H_

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include "definition.h"

/// class to store one ogg page
/** this class is easy to handle, as it only carries the
 *  data area that starts with "OggS".
 **/
class OggPageInternal;
typedef std::shared_ptr<OggPageInternal> OggPage;

class OggPageInternal : public std::enable_shared_from_this<OggPageInternal> {

protected:
  //! pointer to the packet data
  std::vector<uint8_t> m_dataPtr;

  uint32 m_headerLength;
  uint32 m_bodyLength;
  uint8  m_streamNo;
  bool   m_empty;

public:

  OggPageInternal();
  OggPageInternal(std::vector<uint8_t>& data, uint32 headerLength, uint32 bodyLength);
  ~OggPageInternal();


  //! Is this page continued ?
  bool     isContinued();

  //! Is this page a "Begin of Stream" page ?
  bool     isBOS();

  //! Is this page an "End of Stream" page ?
  /*! Every stream within a file (e.g. audio stream and video stream)
    has it's own eos flag */
  bool     isEOS();

  bool     isEmpty();

  void     setContinued();

  void     setEOS();
  void     unsetEOS();

  void     setBOS();
  void     unsetBOS();

  /* what ogg version is this stream */
  uint32   version();
  uint32   packets();
  int64    granulepos();
  uint32   serialno();
  uint32   pageno();

  void     createCRC();

  uint8    getStreamNo();
  void     setStreamNo(uint8 streamNo);

  uint32   length();
  std::vector<uint8_t>&   data();
  uint32   getHeaderLength() {
    return m_headerLength;
  }

  OggPage  clone();
  OggPage  create(std::vector<uint8_t>& data, uint32_t headerLength, uint32_t bodyLength);
  OggPage  getPtr();

  std::string toString(uint8 level);

};



#endif /*OGGPAGE_H_*/
