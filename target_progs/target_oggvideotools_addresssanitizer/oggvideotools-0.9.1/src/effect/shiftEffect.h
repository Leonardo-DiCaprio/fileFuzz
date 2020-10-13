/*
 * shiftEffect.h
 *
 *  Created on: 16.03.2014
 *      Author: seger
 */

#ifndef SHIFTEFFECT_H_
#define SHIFTEFFECT_H_

#include "effector.h"

class ShiftEffect: public Effector {
public:
  class ShiftConfig {
  public:
    bool first;

    uint32 sequenceLength; /* in frames */
    uint32 blindLength;    /* in frames */

    uint32 outputWidth;
    uint32 outputHeight;

    RGBPlane origPlane;
  };

protected:
  RGBPlane presentationPlane;

  enum State {
    unconfigured,
    shifting,
    presentation,
    unavailable
  };

  State state;

  RGBPlane lastPlane;
  RGBPlane shiftPlane;
  uint32 framecounter;

  ShiftConfig config;

  void doShift(RGBPlane& plane);
  void doPresentation(RGBPlane& plane);


public:
  ShiftEffect();
  virtual ~ShiftEffect();

  void configure(ShiftConfig& config);

  virtual Effector& operator>>(RGBPlane& plane);

  virtual bool available();

  virtual void accept(EffectorVisitor& visitor) const;

};

#endif /* SHIFTEFFECT_H_ */
