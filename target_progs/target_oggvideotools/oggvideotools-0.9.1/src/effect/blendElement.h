//
// C++ Interface: blendElement
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef BLENDELEMENT_H
#define BLENDELEMENT_H

#include <string>

#include "rgbPlane.h"

/**
	@author Yorn <yorn@gmx.net>
*/
class BlendElement {

public:

  enum BlendState {
    blend_off,
    blend_slideIn,
    blend_on,
    blend_slideOut,
    blend_end
  };

  std::string pictureName;
  RGBPlane    picture;
  double      startTime;
  double      endTime;
  bool        smooth;
  bool        unavailable;

  BlendState  state;
  float       intensity;

  BlendElement();

  BlendElement ( const std::string& pictureName, double _startTime, double _endTime,
                 bool _smooth );

  std::string getPictureName() const;
  void setPicturePlane(const RGBPlane& plane);
  void setUnavailable();

  virtual ~BlendElement();
};

#endif
