#ifndef definition_h
#define definition_h

#ifdef HAVE_CONFIG_H
#include "config.h"
//#else
// #ifndef NOAUTOTOOLS
//  #warning only use with autotools
// #endif
#endif

/* for now this is static, we do not have a configure script yet */

#ifdef HAVE_STDINT_H

#include "stdint.h"

typedef uint64_t     uint64;
typedef uint32_t     uint32;
typedef uint16_t     uint16;
typedef uint8_t      uint8;
typedef int64_t      int64;
typedef int32_t      int32;
typedef int16_t      int16;
typedef int8_t       int8;

#else

#ifndef HAVE_CCPP
typedef unsigned long long uint64;
typedef unsigned int       uint32;
typedef unsigned short     uint16;
typedef unsigned char      uint8;
typedef long long          int64;
typedef int                int32;
typedef short              int16;
typedef char               int8;
#else
#include <cc++/config.h>
#endif

#endif

#ifndef HAVE_LIBOGG

struct ogg_packet {
  uint8*  packet;
  int32   bytes;
  uint32  b_o_s;
  uint32  e_o_s;

  int64   granulepos;

  int64   packetno;
};

#else

#include <ogg/ogg.h>

#endif

/* function to convert a 32 Bit value from little to big endian
 * and vice versa */
inline uint32 convert32(uint32 in)
{
  uint32 out;
  char* byteOut = (char*)(&out);
  char* byteIn  = (char*)(&in);

  byteOut[0] = byteIn[3];
  byteOut[1] = byteIn[2];
  byteOut[2] = byteIn[1];
  byteOut[3] = byteIn[0];
  return (out);
}

/* function to convert a 24 Bit value from little to big endian
 * and vice versa */
inline uint32 convert24(uint32 in)
{
  uint32 out(0);
  char* byteOut = (char*)(&out);
  char* byteIn  = (char*)(&in);

  byteOut[0] = byteIn[2];
  byteOut[1] = byteIn[1];
  byteOut[2] = byteIn[0];
  return (out);
}

/* function to convert a 16 Bit value from little to big endian
 * and vice versa */
inline uint16 convert16(uint16 in)
{
  uint16 out;
  char* byteOut = (char*)(&out);
  char* byteIn  = (char*)(&in);

  byteOut[0] = byteIn[1];
  byteOut[1] = byteIn[0];
  return (out);
}

#endif
