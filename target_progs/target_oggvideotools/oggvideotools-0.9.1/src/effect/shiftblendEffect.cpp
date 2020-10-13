/*
 * shiftEffect.cpp
 *
 *  Created on: 16.03.2014
 *      Author: seger
 */

#include "shiftblendEffect.h"

#include <iostream>
#include <cstring>
#include <cmath>

#include "pictureBlend.h"
#include "pictureResize.h"
#include "log.h"
#include "effectorVisitor.h"

ShiftblendEffect::ShiftblendEffect()
  :state(unconfigured), framecounter(0)
{

}

ShiftblendEffect::~ShiftblendEffect()
{

}

bool ShiftblendEffect::available()
{
  return((state!=unavailable) && (state!=unconfigured));
}

void ShiftblendEffect::accept(EffectorVisitor& visitor) const
{
  visitor.visit(*this);
}

Effector& ShiftblendEffect::operator >>(RGBPlane& plane)
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

void ShiftblendEffect::configure(ShiftConfig& _config)
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

  RGBPlane blackplane_full_alpha(config.outputWidth, config.outputHeight, 0x7f7f7f7f );

  if (config.first)
    state = presentation;
  else {
    logger.debug() << "Shifting -- \n";
//	  shiftPlane = PictureResize::concatenate(lastPlane, blackplane_full_alpha, lastPlane);
    shiftPlane = PictureResize::concatenate(lastPlane, blackplane_full_alpha, lastPlane);
    logger.debug() << "Shifting -- (" << shiftPlane.getWidth()<<":"<<shiftPlane.getHeight()<<") \n";
    state = shifting;
  }

}

void ShiftblendEffect::doShift(RGBPlane& plane)
{

  // linear

  static const float pi = 3.14159265359;
  //
  float fact((framecounter*1.0)/(config.blindLength*1.0)*pi + pi);
  float factor = (1.0 + cos(fact))/2.0;

  logger.debug() << "  Shifting:"<< factor <<" \n";
  RGBPlane tmpframe = PictureResize::subframe(shiftPlane, config.outputWidth, config.outputHeight, ((float)config.outputWidth)*factor, 0, 1);

  plane = PictureBlend::alphaBlend(presentationPlane, tmpframe,  1-factor);

  logger.debug() << "  Subframe -- (" << plane.getWidth()<<":"<<plane.getHeight()<<") \n";

  framecounter++;
  if (framecounter > config.blindLength) {
    logger.debug() << "Presenting -- \n";
    state = presentation;
  }

}

void ShiftblendEffect::doPresentation(RGBPlane& plane)
{

  plane = presentationPlane;

  framecounter++;
  if (framecounter > config.sequenceLength) {
    lastPlane = presentationPlane;

    state = unavailable;
  }

}


