//
// C++ Interface: basePlane
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef BASEPLANE_H
#define BASEPLANE_H

#include "definition.h"

/**
BasePlane contains the picture information (width, height and picture plane)


	@author Yorn <yorn@gmx.net>
*/
class BasePlane {

public:
  uint32 width;
  uint32 height;

  uint8*      plane;


  BasePlane();

  BasePlane(uint32 _width, uint32 _height, uint32 color = 0x00000000 );

  virtual ~BasePlane();

};

#endif
