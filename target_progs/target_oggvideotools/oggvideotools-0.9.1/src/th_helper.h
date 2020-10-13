#ifndef TH_HELPER_H
#define TH_HELPER_H

//#ifdef HAVE_THEORAENC || HAVE_THEORADEC

#include <cstring>
#include <theora/codec.h>

#include "definition.h"

inline void th_clean_ycbcr(th_ycbcr_buffer& theoraPictureBuffer)
{
#ifdef HAVE_BZERO
  bzero(theoraPictureBuffer, sizeof(th_img_plane)*3);
#else
  // with the right alignment, this could be much faster ... I hope for the compiler to do that :-/
  for (int i(0); i<sizeof(th_img_plane)*3; ++i) *(((uint8*)theoraPictureBuffer)+i) = 0;
#endif

}

inline void th_free_ycbcr(th_ycbcr_buffer& theoraPictureBuffer)
{
  // kind of ugly

  delete[] theoraPictureBuffer[0].data;
  delete[] theoraPictureBuffer[1].data;
  delete[] theoraPictureBuffer[2].data;

  th_clean_ycbcr(theoraPictureBuffer);
}

//#endif

#endif