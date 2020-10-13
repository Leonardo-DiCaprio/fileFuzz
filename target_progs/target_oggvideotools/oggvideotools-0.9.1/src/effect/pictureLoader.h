//
// C++ Interface: pictureLoader
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PICTURELOADER_H
#define PICTURELOADER_H

#include <string>

#include "rgbPlane.h"

#ifdef WITH_GD2LIB
#include <gd.h>
#endif

#ifdef HAVE_LIBTHEORAENC
#include <theora/codec.h>
#endif

/**
	@author Yorn <yorn@gmx.net>
*/
class PictureLoader {

protected:
  enum SuffixType {
    suffix_unknown,
    suffix_jpg,
    suffix_png,
    suffix_gif
  };

#ifdef WITH_GD2LIB
  static RGBPlane convertToRgbPlane(gdImagePtr im);
  static SuffixType identifySuffix(const std::string& filename);
#endif

public:

  PictureLoader();

  virtual ~PictureLoader();

#ifdef WITH_GD2LIB
  static bool load(RGBPlane& retPlane, const std::string& filename, uint32 width=0, uint32 height=0, bool useBiggest = true);
  static bool save(RGBPlane& pic, const std::string& filename, uint32 width=0, uint32 height=0);
#endif

#ifdef HAVE_LIBTHEORAENC
  static void exportYCrCb_theora(RGBPlane& plane, th_ycbcr_buffer& buffer, int pixel_format=TH_PF_420);
  static void exportYCrCb_444_theora(RGBPlane& picture, th_ycbcr_buffer& buffer);

  static RGBPlane importYCrCb_theora(const th_ycbcr_buffer& buffer, uint32 width, uint32 height, uint32 XOffset=0, uint32 YOffset=0, int pixel_format=TH_PF_420);
#endif


};

#endif
