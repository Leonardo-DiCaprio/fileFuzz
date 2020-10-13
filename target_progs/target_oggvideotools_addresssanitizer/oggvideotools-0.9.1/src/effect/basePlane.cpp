//
// C++ Implementation: basePlane
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "basePlane.h"

BasePlane::BasePlane()
  : width(0), height(0), plane(0)
{
}

BasePlane::BasePlane(uint32 _width, uint32 _height, uint32 color)
  : width(_width), height(_height), plane(new uint8[_height*_width*4])
{
  uint32* ptr((uint32*)plane);
  for (uint32 i(0); i<(_height*_width); ++i)
    ptr[i] = color;
}

BasePlane::~BasePlane()
{
  delete[] plane;
}
