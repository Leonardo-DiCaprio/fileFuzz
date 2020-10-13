//
// C++ Interface: lowpassEffect
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LOWPASSEFFECT_H
#define LOWPASSEFFECT_H

#include "effector.h"

/**
	@author Yorn <yorn@gmx.net>
*/
class LowpassEffect : public Effector {

public:
  class LowPassPictureConfig {
  public:

    bool first;
    bool last;

    uint32 sequenceLength; /* in frames */
    uint32 blindLength;    /* in frames */

    uint32 outputWidth;
    uint32 outputHeight;

    RGBPlane origPlane;
  };

protected:

  enum State {
    unconfigured,
    blindIn,
    presentation,
    blindOut,
    unavailable
  };

  State state;
  RGBPlane presentationPlane;
  uint32 framecounter;
  float factor;

  LowPassPictureConfig config;

  void doBlindIn(RGBPlane& plane);
  void doBlindOut(RGBPlane& plane);
  void doPresentation(RGBPlane& plane);

public:
  LowpassEffect();
  virtual ~LowpassEffect();

  void configure(LowPassPictureConfig& config);

  virtual Effector& operator>>(RGBPlane& plane);

  virtual bool available();

  virtual void accept(EffectorVisitor& visitor) const;

};


#endif
