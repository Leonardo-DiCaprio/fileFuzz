/*
 * ShiftblendEffect.h
 *
 *  Created on: 16.03.2014
 *      Author: seger
 */

#ifndef SHIFTBLENDEFFECT_H_
#define SHIFTBLENDEFFECT_H_

#include "effector.h"

class ShiftblendEffect: public Effector {
public:
  class ShiftConfig {
  public:
    bool first;

    uint32 sequenceLength; /* in frames */
    uint32 blindLength;    /* in frames */

    uint32 outputWidth;
    uint32 outputHeight;

    enum Type {
      Left,
      Right
    };

    Type type;

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
  ShiftblendEffect();
  virtual ~ShiftblendEffect();

  void configure(ShiftConfig& config);

  virtual Effector& operator>>(RGBPlane& plane);

  virtual bool available();

  virtual void accept(EffectorVisitor& visitor) const;

};

#endif /* ShiftblendEffect_H_ */
