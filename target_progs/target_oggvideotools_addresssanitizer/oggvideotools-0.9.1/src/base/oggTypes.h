/*
 * this emun should carry all known streams that could be inserted into
 * the ogg container
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

#ifndef OGGTYPES_H_
#define OGGTYPES_H_

#include <cstdint>
#include <type_traits>

#if __GNUC__ > 4 || \
              (__GNUC__ == 4 && (__GNUC_MINOR__ > 9 || \
                                 (__GNUC_MINOR__ == 9 && \
                                  __GNUC_PATCHLEVEL__ > 2)))

template<typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type {
  return static_cast<typename std::underlying_type<E>::type>(e);
}
#else
#define to_integral(a) (int)(a)
#endif

#define MAXIDCHARS 7

enum class OggType {
  unknown,
  vorbis,
  theora,
  kate,
  maxOggType
};

static const uint8_t OggTypeMap[to_integral(OggType::maxOggType)][MAXIDCHARS] = {
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x01, 'v', 'o', 'r', 'b', 'i', 's' },
  { 0x80, 't', 'h', 'e', 'o', 'r', 'a' },
  { 0x80, 'k', 'a', 't', 'e', 0x00, 0x00 }
};

#endif /*OGGTYPES_H_*/
