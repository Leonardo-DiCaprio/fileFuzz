//
// C++ Interface: rgbPlane
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef RGBPLANE_H
#define RGBPLANE_H

#include "refObject.h"
#include "basePlane.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/**
	@author Yorn <yorn@gmx.net>
*/
class RGBPlane : public RefObject<BasePlane> {

public:
  RGBPlane();
  RGBPlane(uint32 width, uint32 height, uint32 color = 0x00000000 );

  virtual ~RGBPlane();

  const uint32 getWidth() const {
    return objPtr->width;
  }
  const uint32 getHeight() const {
    return objPtr->height;
  }

};

#endif
