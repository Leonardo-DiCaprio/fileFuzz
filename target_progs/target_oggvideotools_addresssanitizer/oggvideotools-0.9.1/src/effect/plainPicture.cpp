//
// C++ Implementation: PlainPicture
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <iostream>
#include "plainPicture.h"
#include "pictureResize.h"
#include "pictureLoader.h"
#include <cstdlib>
#include "log.h"
#include "effectorVisitor.h"

PlainPicture::PlainPicture()
  : Effector(), state(unconfigured), framecounter(0)
{
}


PlainPicture::~PlainPicture()
{
}

Effector & PlainPicture::operator >>(RGBPlane & plane)
{

  if (!available())
    return(*this);

  plane = presentationPlane;

  framecounter++;
  if (framecounter > config.sequenceLength) {
    state = unavailable;
  }

  return(*this);
}

void PlainPicture::configure(PlainPictureConfig & _config)
{

  framecounter = 0;
  config = _config;

  logger.debug() << "PlanePicture::configure: "<< config.origPlane->width << "x" << config.origPlane->height<<" -> "
                 <<config.outputWidth<<"x"<<config.outputHeight<<std::endl;

  /* resize the picture to the correct size */
  if ((config.origPlane->width != config.outputWidth) || (config.origPlane->height != config.outputHeight)) {
    logger.debug() << "reframing"<<std::endl;
    presentationPlane = PictureResize::reframe(config.origPlane, config.outputWidth, config.outputHeight);
  } else
    presentationPlane = config.origPlane;

  logger.debug() << "new Picture: "<< presentationPlane->width << "x"<<presentationPlane->height<<std::endl;
  state = presentation;

}

bool PlainPicture::available()
{

  return((state!=unavailable) && (state!=unconfigured));

}

void PlainPicture::accept(EffectorVisitor& visitor) const
{
  visitor.visit(*this);
}
