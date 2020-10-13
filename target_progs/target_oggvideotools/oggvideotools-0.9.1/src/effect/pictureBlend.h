//
// C++ Interface: pictureBlend
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PICTUREBLEND_H
#define PICTUREBLEND_H

#include "rgbPlane.h"
/**
	@author Yorn <yorn@gmx.net>
*/
class PictureBlend {

public:
  PictureBlend();

  virtual ~PictureBlend();

  static RGBPlane alphaBlend(RGBPlane& picture, RGBPlane& alphaPic, float intensity);

  static RGBPlane crossfade(RGBPlane& plane1, RGBPlane& plane2, float velocity);

};

#endif
