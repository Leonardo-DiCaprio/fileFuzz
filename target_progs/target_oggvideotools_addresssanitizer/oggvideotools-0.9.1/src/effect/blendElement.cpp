//
// C++ Implementation: blendElement
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <iostream>

#include "blendElement.h"
#include "pictureLoader.h"

BlendElement::BlendElement()
  :    startTime ( 0 ), endTime ( -1 ),
       smooth ( false ), unavailable(true), state ( blend_off ), intensity ( 0.0 )
{
}

BlendElement::BlendElement ( const std::string& _pictureName, double _startTime, double _endTime,
                             bool _smooth ) :
  pictureName ( _pictureName ), startTime ( _startTime ), endTime ( _endTime ),
  smooth ( _smooth ), unavailable(true), state ( blend_off ), intensity ( 0.0 )
{
}


BlendElement::~BlendElement()
{
}

std::string BlendElement::getPictureName() const
{
  return (pictureName);
}

void BlendElement::setPicturePlane(const RGBPlane& _picture )
{
  picture = _picture;
  unavailable = false;
}

