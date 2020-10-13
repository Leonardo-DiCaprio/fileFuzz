//
// C++ Interface: kenburnseffect
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KENBURNSEFFECT_H
#define KENBURNSEFFECT_H

#include "effector.h"

/**
	@author Yorn <yorn@gmx.net>
*/
class KenBurnsEffect : public Effector {

public:
  class KenBurnsConfig {
  public:
    bool first;
    bool last;

    uint32 sequenceLength; /* in frames */
    uint32 blindLength;    /* in frames */

    uint32 outputWidth;
    uint32 outputHeight;

    float startpointX;
    float startpointY;
    float endpointX;
    float endpointY;
    float zoomStart;
    float zoomEnd;

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

  KenBurnsConfig config;
  uint32 frameCounter;

  State state;

  RGBPlane blackPlane;
  RGBPlane presentationPlane;

  float stepX;
  float stepY;
  float stepZoom;

  float actX;
  float actY;
  float actZoom;

  void doBlindIn(RGBPlane& plane);
  void doPresentation(RGBPlane& plane);
  void doBlindOut(RGBPlane& plane);

public:
  KenBurnsEffect();
  ~KenBurnsEffect();

  void configure(KenBurnsConfig& config);

  virtual Effector& operator>>(RGBPlane& plane);
  virtual bool available();
  virtual void accept(EffectorVisitor& visitor) const;

  static KenBurnsConfig createKBconfigRandom(RGBPlane& plane, uint32 pictureWidth, uint32 pictureHeight, uint32 frameWidth, uint32 frameHeight, uint32 sequenceLength, uint32 blindLength);
  static KenBurnsConfig createKBconfigPredefine(RGBPlane& plane, uint32 pictureWidth, uint32 pictureHeight, uint32 frameWidth, uint32 frameHeight, uint32 sequenceLength, uint32 blindLength, uint32 predefine);
};

#endif
