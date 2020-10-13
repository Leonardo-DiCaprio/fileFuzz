//
// C++ Interface: pictureResize
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PICTURERESIZE_H
#define PICTURERESIZE_H

#include "definition.h"
#include "rgbPlane.h"

/**
	@author Yorn <yorn@gmx.net>
*/
class PictureResize {

protected:
  static uint32 calculateKernelValue(RGBPlane& pic, float posX, float posY, float radius, bool p=false);
  static float getWeight(float distance, float radius);

  static uint32 calculateKernelValueFix(RGBPlane& pic, float posX, float posY, float radius, bool p=false);
  static int32 getWeightFix(uint32 distance, uint32 radius);

  static uint32 linearInterpolation(RGBPlane pic, float x, float y);
public:
  PictureResize();

  virtual ~PictureResize();

  //! Method transforms the original picture through a lowpass/bluring filter
  static RGBPlane kernelLowpass(RGBPlane& picture, float radius=1);

  //! method resizes the picture and stretches if necessary
  static RGBPlane resize(RGBPlane& picture, uint32 width, uint32 height, uint8 quality=1);

  static RGBPlane resize(RGBPlane& picture, float factorX, float factorY, uint8 quality=1);

  static RGBPlane resize(RGBPlane& picture, float factor, uint8 quality=1);

  //! Method keeps the aspect ratio during resize
  static RGBPlane reframe(RGBPlane& picture, uint32 width, uint32 height, uint8 quality=1, uint32 background=0, double aspectCorrection=1);

  static RGBPlane reframe_fixed(RGBPlane & picture, uint32 width, uint32 height, uint32 background);

  static RGBPlane subframe(RGBPlane& picture, uint32 newWidth, uint32 newHeight, float offsetWidth, float offsetHeight, float resizeFactor, uint8 quality=1);

  static RGBPlane concatenate(RGBPlane& picture1, RGBPlane& picture2, RGBPlane& picture3);

};

#endif
