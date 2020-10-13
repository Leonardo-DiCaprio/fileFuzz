/*
 * shiftEffect.cpp
 *
 *  Created on: 16.03.2014
 *      Author: seger
 */

#include "shiftEffect.h"

#include <iostream>
#include <cstring>
#include <cmath>

#include "pictureBlend.h"
#include "pictureResize.h"
#include "log.h"
#include "effectorVisitor.h"

ShiftEffect::ShiftEffect()
  :state(unconfigured), framecounter(0)
{

}

ShiftEffect::~ShiftEffect()
{

}

bool ShiftEffect::available()
{
  return((state!=unavailable) && (state!=unconfigured));
}

void ShiftEffect::accept(EffectorVisitor& visitor) const
{
  visitor.visit(*this);
}

Effector& ShiftEffect::operator >>(RGBPlane& plane)
{

  switch (state) {

  case shifting: {
    doShift(plane);
    break;
  }

  case presentation: {
    doPresentation(plane);
    break;
  }

  default: {
    logger.error() << "no frame available\n";
    break;
  }
  }

  return(*this);

}

void ShiftEffect::configure(ShiftConfig& _config)
{

  framecounter = 0;

  config = _config;

  if (config.first) {

    lastPlane = RGBPlane(config.outputWidth, config.outputHeight);

    /* blank the plane */
    uint32 planesize = config.outputWidth*config.outputHeight*4; // 3 Colors + Alpha channel
    memset(lastPlane->plane, 0x00, planesize);

  }


  /* resize the picture to the correct size */
  presentationPlane = PictureResize::reframe(config.origPlane, config.outputWidth, config.outputHeight);

  logger.debug() << "Picture size: "<< presentationPlane->width<<" x "<<presentationPlane->width
                 <<"  -> frame size "<<config.outputWidth <<" x "<<config.outputHeight<<std::endl;

  if (config.first)
    state = presentation;
  else {
    logger.debug() << "Shifting -- \n";
    shiftPlane = PictureResize::concatenate(lastPlane, presentationPlane, lastPlane);
    logger.debug() << "Shifting -- (" << shiftPlane.getWidth()<<":"<<shiftPlane.getHeight()<<") \n";
    state = shifting;
  }
}

void ShiftEffect::doShift(RGBPlane& plane)
{

  // linear
  // float factor((framecounter*1.0)/(config.blindLength*1.0));
  static const float pi = 3.14159265359;
  //
  float fact((framecounter*1.0)/(config.blindLength*1.0)*pi + pi);
  float factor = (1.0 + cos(fact))/2.0;

  logger.debug() << "  Shifting:"<< factor <<" \n";
  plane = PictureResize::subframe(shiftPlane, config.outputWidth, config.outputHeight, ((float)config.outputWidth)*factor, 0, 1);

  logger.debug() << "  Subframe -- (" << plane.getWidth()<<":"<<plane.getHeight()<<") \n";

  framecounter++;
  if (framecounter > config.blindLength) {
    logger.debug() << "Presenting -- \n";
    state = presentation;
  }

}

void ShiftEffect::doPresentation(RGBPlane& plane)
{

  plane = presentationPlane;

  framecounter++;
  if (framecounter > config.sequenceLength) {
    lastPlane = presentationPlane;

    state = unavailable;
  }

}


