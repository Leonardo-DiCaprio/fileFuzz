//
// C++ Interface: plainPicture
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PLAINPICTURE_H
#define PLAINPICTURE_H

#include "effector.h"

/**
	@author Yorn <yorn@gmx.net>
*/
class PlainPicture : public Effector {

public:

  class PlainPictureConfig {
  public:

    uint32 sequenceLength; /* in frames */

    uint32 outputWidth;
    uint32 outputHeight;

    RGBPlane origPlane;
  };

protected:

  enum State {
    unconfigured,
    presentation,
    unavailable
  };

  State state;
  RGBPlane presentationPlane;
  PlainPictureConfig config;
  uint32 framecounter;


public:
  PlainPicture();
  virtual ~PlainPicture();

  void configure(PlainPictureConfig& config);

  virtual Effector& operator>>(RGBPlane& plane);

  virtual bool available();

  virtual void accept(EffectorVisitor& visitor) const;

};

#endif
