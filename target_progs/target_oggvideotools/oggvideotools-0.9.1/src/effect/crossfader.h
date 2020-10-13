//
// C++ Interface: crossfader
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CROSSFADER_H
#define CROSSFADER_H

#include "effector.h"

/**
	@author Yorn <yorn@gmx.net>
*/
class Crossfader : public Effector {
public:
  class CrossfaderConfig {
  public:
    bool first;

    uint32 sequenceLength; /* in frames */
    uint32 blindLength;    /* in frames */

    uint32 outputWidth;
    uint32 outputHeight;

    RGBPlane origPlane;
  };

  RGBPlane presentationPlane;

protected:
  enum State {
    unconfigured,
    crossfade,
    presentation,
    unavailable
  };

  State state;

  RGBPlane lastPlane;
  uint32 framecounter;

  CrossfaderConfig config;

  void doCrossfade(RGBPlane& plane);
  void doPresentation(RGBPlane& plane);

public:
  Crossfader();
  ~Crossfader();

  void configure(CrossfaderConfig& config);

  virtual Effector& operator>>(RGBPlane& plane);

  virtual bool available();

  virtual void accept(EffectorVisitor& visitor) const;

};

#endif
