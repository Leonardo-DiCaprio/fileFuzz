//
// C++ Implementation: crossfader
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "crossfader.h"

#include <iostream>
#include <cstring>

#include "pictureBlend.h"
#include "pictureResize.h"
#include "log.h"
#include "effectorVisitor.h"

Crossfader::Crossfader()
  : Effector(), state(unconfigured), framecounter(0)
{
}


Crossfader::~Crossfader()
{
}

void Crossfader::configure(CrossfaderConfig& _config)
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

  logger.debug() << "Picture size: "<< presentationPlane->width<<" x "<<presentationPlane->height
                 <<"  -> frame size "<<config.outputWidth <<" x "<<config.outputHeight<<std::endl;

  if (config.first)
    state = presentation;
  else
    state = crossfade;

}

void Crossfader::doCrossfade(RGBPlane & plane)
{

  plane = PictureBlend::crossfade(lastPlane, presentationPlane,(framecounter*1.0)/(config.blindLength*1.0));

  framecounter++;
  if (framecounter > config.blindLength) {
    state = presentation;
  }

}

void Crossfader::doPresentation(RGBPlane & plane)
{

  plane = presentationPlane;

  framecounter++;
  if (framecounter > config.sequenceLength) {
    lastPlane = presentationPlane;

    state = unavailable;
  }

}

bool Crossfader::available()
{
  return((state!=unavailable) && (state!=unconfigured));
}

void Crossfader::accept(EffectorVisitor& visitor) const
{
  visitor.visit(*this);
}

Effector & Crossfader::operator >>(RGBPlane & plane)
{

  switch (state) {

  case crossfade: {
    doCrossfade(plane);
    break;
  }

  case presentation: {
    doPresentation(plane);
    break;
  }

  default: {
    logger.error() << "KenBurnsEffect: no frame available\n";
    break;
  }
  }

  return(*this);
}
