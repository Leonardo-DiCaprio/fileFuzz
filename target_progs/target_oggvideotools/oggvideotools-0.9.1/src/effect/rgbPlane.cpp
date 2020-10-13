//
// C++ Implementation: rgbPlane
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "rgbPlane.h"


RGBPlane::RGBPlane()
{
}


RGBPlane::~RGBPlane()
{
}

RGBPlane::RGBPlane(uint32 width, uint32 height, uint32 color)
  : RefObject<BasePlane>(new BasePlane(width, height, color))
{
}


