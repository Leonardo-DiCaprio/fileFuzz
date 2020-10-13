//
// C++ Implementation: lowpassEffect
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "lowpassEffect.h"

#include <iostream>
#include <cmath>

#include "pictureResize.h"
#include "log.h"
#include "effectorVisitor.h"

LowpassEffect::LowpassEffect()
  : state(unconfigured)
{
}


LowpassEffect::~LowpassEffect()
{
}

void LowpassEffect::configure(LowPassPictureConfig & _config)
{

  config = _config;
  framecounter = 0;

  factor = 0.5;

  presentationPlane = PictureResize::reframe(config.origPlane, config.outputWidth, config.outputHeight);

  if (config.first)
    state = presentation;
  else
    state = blindIn;
}

Effector & LowpassEffect::operator >>(RGBPlane & plane)
{
  switch (state) {

  case blindIn: {
    doBlindIn(plane);
    break;
  }

  case blindOut: {
    doBlindOut(plane);
    break;
  }

  case presentation: {
    doPresentation(plane);
    break;
  }

  default: {
    logger.error() << "LowpassEffect: no frame available\n";
    break;
  }
  }


}

void LowpassEffect::doBlindIn(RGBPlane & plane)
{
  uint32 n = (config.blindLength - framecounter);

  float filterValue = n*1.0/(config.blindLength*1.0);//1.0/(2.0+(n*1.0/(config.blindLength*1.0)*100.0)); //powf(factor,n);
//  logger.debug() << " -- blindin - fr "<< framecounter <<"   fa "<<factor << "  n "<<n <<" fw "<<filterValue<< "     ";

  plane = PictureResize::kernelLowpass(presentationPlane, 1.0-filterValue);//lowpassFilter(filterValue);

  framecounter++;
  if (framecounter > config.blindLength) {
    state = presentation;
  }

}

void LowpassEffect::doPresentation(RGBPlane & plane)
{

  plane = presentationPlane; //.reframe(config.outputWidth, config.outputHeight);

  framecounter++;
  if (framecounter > (config.sequenceLength - config.blindLength)) {
    if (!config.last) {
      state = blindOut;
    } else {
      if (framecounter >= config.sequenceLength) {
        state = unavailable;
      }
    }

  }

}

void LowpassEffect::doBlindOut(RGBPlane & plane)
{
  uint32 n = (framecounter - (config.sequenceLength - config.blindLength));

  float filterValue = n*1.0/(config.blindLength*1.0);//1.0/(2.0+(n*1.0/(config.blindLength*1.0)*100.0)); //powf(factor,n);

  logger.debug() << " -- blindout - fr "<< framecounter <<"   fa "<<factor << "  n "<<n <<" fw "<<filterValue<<"     ";

  plane = PictureResize::kernelLowpass(presentationPlane, 1.0-filterValue);//lowpassFilter(filterValue);

  framecounter++;
  if (framecounter >= config.sequenceLength) {
    state = unavailable;
  }

}

bool LowpassEffect::available()
{
  return((state != unavailable) && (state != unconfigured));
}

void LowpassEffect::accept(EffectorVisitor& visitor) const
{
  visitor.visit(*this);
}
