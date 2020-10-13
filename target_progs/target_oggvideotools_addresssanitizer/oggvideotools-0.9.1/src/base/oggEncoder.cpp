/*
 * OggEncoder
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

#include "oggEncoder.h"
#include "log.h"

PageBufferElement::PageBufferElement()
  : time(-1), empty(true), interpreter(0)
{
}

PageBufferElement::~PageBufferElement()
{
}

OggEncoder::OggEncoder()
  : withBuffer(false)
{
  setConfigured();
}

OggEncoder::~OggEncoder()
{
}

/*
void OggEncoder::createJitterBuffer(uint8 numOfStreams)
{
  if (numOfStreams) {
    oggBuffer.resize(numOfStreams);
    withBuffer = true;
  }
}
*/

void OggEncoder::flush()
{

}

void OggEncoder::serializePage(OggPage& page)
{

  rawPacketList.push_back(std::make_shared<RawMediaPacketInternal>(page->data(), true));

  setAvailable();
}

void OggEncoder::insertNextPage(OggPage& page)
{

}


OggEncoder& OggEncoder::operator<<(OggPage page)
{
  if (!withBuffer) {
    serializePage(page);
    return(*this);
  }
  /*
      uint8  m_streamNo(page->getStreamNo());
      double time(oggBuffer[m_streamNo].interpreter->getTime(page->granulepos()));

      double nextTime(-2);
      uint8  nextStreamNo(0);

      while(!oggBuffer[m_streamNo].m_empty) {

        // find the next packet to send out
        for(uint32 i(0); i<oggBuffer.size(); ++i) {
          if (oggBuffer[i].m_empty)
            continue;
          if ((nextTime < -1) || (nextTime > oggBuffer[i].time)) {
            nextTime = oggBuffer[i].time;
            nextStreamNo = i;
          }
        }

        // write page to the output buffer
        serializePage(oggBuffer[nextStreamNo].page);

        // ... and mark the buffer as m_empty
        oggBuffer[nextStreamNo].m_empty = true;

      }

    // insert the page into the buffer
    oggBuffer[m_streamNo].page     = page;
    oggBuffer[m_streamNo].time     = time;
    oggBuffer[m_streamNo].m_empty    = false;
  */
  return(*this);
}

OggEncoder& OggEncoder::operator>>(RawMediaPacket& packet)
{
  if (isEmpty()) {
    logger.error() << "OggEncoder::operator>>: no packet available\n";
    return(*this);
  }

  packet = rawPacketList.front();
  rawPacketList.pop_front();

  if (rawPacketList.empty())
    setEmpty();

  return(*this);
}


