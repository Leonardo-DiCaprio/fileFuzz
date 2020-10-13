//
// C++ Implementation: kenburnseffect
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "kenburnseffect.h"

#include <iostream>
#include <cmath>
#include <cstring>
#include <cstdlib>

#include "pictureResize.h"
#include "pictureBlend.h"
#include "log.h"
#include "effectorVisitor.h"

KenBurnsEffect::KenBurnsEffect() :
  Effector(), state(unconfigured)
{
}

KenBurnsEffect::~KenBurnsEffect()
{
}

void KenBurnsEffect::configure(KenBurnsEffect::KenBurnsConfig& _config)
{

  config = _config;

  stepX = ((config.endpointX - config.startpointX) * 1.0)
          / (config.sequenceLength * 1.0);
  stepY = ((config.endpointY - config.startpointY) * 1.0)
          / (config.sequenceLength * 1.0);
  stepZoom = ((config.zoomEnd - config.zoomStart) * 1.0)
             / (config.sequenceLength * 1.0);

  actX = config.startpointX;
  actY = config.startpointY;
  actZoom = config.zoomStart;

  blackPlane = RGBPlane(config.outputWidth, config.outputHeight);
  presentationPlane = config.origPlane;

  /* blank the plane */
  uint32 planesize = config.outputWidth * config.outputHeight * 4; // 3 Colors + Alpha channel
  memset(blackPlane->plane, 0x00, planesize);

  frameCounter = 0;

  if (config.first)
    state = presentation;
  else
    state = blindIn;

}

void KenBurnsEffect::doBlindIn(RGBPlane& plane)
{

  logger.debug() << "   --- Position " << actX <<" x "<< actY <<"    "<<( 1.0/actZoom);

  // get the picture to be fade in
  plane = PictureResize::subframe(presentationPlane, config.outputWidth,
                                  config.outputHeight, actX, actY, 1.0 / actZoom);

  // calculate the next fader picture
  plane = PictureBlend::crossfade(blackPlane, plane, (frameCounter * 1.0)
                                  / (config.blindLength * 1.0));

  // let the fading go on
  actX += stepX;
  actY += stepY;
  actZoom += stepZoom;

  frameCounter++;
  if (frameCounter > config.blindLength) {
    state = presentation;
  }
}

void KenBurnsEffect::doPresentation(RGBPlane& plane)
{
  logger.debug() << "   --- Position " << actX <<" x "<< actY <<"    "<<( 1.0/actZoom);

  // get the picture to be fade in
  plane = PictureResize::subframe(presentationPlane, config.outputWidth,
                                  config.outputHeight, actX, actY, 1.0 / actZoom);

  // let the fading go on
  actX += stepX;
  actY += stepY;
  actZoom += stepZoom;

  frameCounter++;
  if (frameCounter > (config.sequenceLength - config.blindLength)) {
    if (!config.last) {
      state = blindOut;
    } else {
      if (frameCounter >= config.sequenceLength) {
        state = unavailable;
      }
    }
  }

}

void KenBurnsEffect::doBlindOut(RGBPlane & plane)
{
#ifdef DEBUG
  logger.debug() << "   --- Position " << actX <<" x "<< actY <<"    "<<( 1.0/actZoom);
#endif
  // get the picture to be fade in
  plane = PictureResize::subframe(presentationPlane, config.outputWidth,
                                  config.outputHeight, actX, actY, 1.0 / actZoom);

  // calculate the next fader picture
  plane = PictureBlend::crossfade(blackPlane, plane, ((config.sequenceLength
                                  - frameCounter) * 1.0) / (config.blindLength * 1.0));

  // let the fading go on
  actX += stepX;
  actY += stepY;
  actZoom += stepZoom;

  frameCounter++;
  if (frameCounter >= config.sequenceLength) {
    state = unavailable;
  }

}

bool KenBurnsEffect::available()
{
  return ((state != unavailable) && (state != unconfigured));
}

void KenBurnsEffect::accept(EffectorVisitor& visitor) const
{
  visitor.visit(*this);
}

Effector & KenBurnsEffect::operator >>(RGBPlane & plane)
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
    logger.error() << "KenBurnsEffect: no frame available\n";
    break;
  }
  }
  return(*this);
}

KenBurnsEffect::KenBurnsConfig KenBurnsEffect::createKBconfigRandom(
  RGBPlane& plane, uint32 pictureWidth, uint32 pictureHeight,
  uint32 frameWidth, uint32 frameHeight, uint32 sequenceLength,
  uint32 blindLength)
{

  KenBurnsConfig config;

  config.origPlane = plane;
  config.outputWidth = frameWidth;
  config.outputHeight = frameHeight;
  config.sequenceLength = sequenceLength;
  config.blindLength = blindLength;

  float maxZoomfactor;
  if ((pictureWidth * 1.0) / (frameWidth * 1.0) * (frameHeight * 1.0)
      < (pictureHeight * 1.0))
    maxZoomfactor = (pictureWidth * 1.0) / (frameWidth * 1.0);
  else
    maxZoomfactor = (pictureHeight * 1.0) / (frameHeight * 1.0);

  config.zoomStart = maxZoomfactor * 0.75 + rand() * (maxZoomfactor * 0.25)
                     / (RAND_MAX * 1.0);
  config.zoomEnd = maxZoomfactor * 0.75 + rand() * (maxZoomfactor * 0.25)
                   / (RAND_MAX * 1.0);

#ifdef DEBUG
  logger.debug() << "Zooming ("<<maxZoomfactor<<") from factor "<<config.zoomStart<<" to "<<config.zoomEnd<<std::endl;
#endif

  float availableXStart = pictureWidth - frameWidth * config.zoomStart;
  float availableYStart = pictureHeight - frameHeight * config.zoomStart;

  float availableXEnd = pictureWidth - frameWidth * config.zoomEnd;
  float availableYEnd = pictureHeight - frameHeight * config.zoomEnd;

  if ((availableXStart < 0) || (availableYStart < 0) || (availableXEnd < 0) || (availableYEnd < 0)) {
    logger.error()<< "KenBurnsSequence: picture too small\n";
    // what should we do on error?
  }

  float availLengthSqr(powf((availableXStart - availableXEnd), 2.0) + powf(
                         (availableYStart - availableYEnd), 2.0));
  float lengthSqr;

  do {
    config.startpointX = (((float) rand()) * availableXStart) / (RAND_MAX
                         * 1.0);
    config.startpointY = (((float) rand()) * availableYStart) / (RAND_MAX
                         * 1.0);

    config.endpointX = (((float) rand()) * availableXEnd)
                       / (RAND_MAX * 1.0);
    config.endpointY = (((float) rand()) * availableYEnd)
                       / (RAND_MAX * 1.0);

    // calculate walklength
    float lengthX = fabs(config.startpointX - config.endpointX);
    float lengthY = fabs(config.startpointY - config.endpointY);

    lengthSqr = powf(lengthX, 2.0) + powf(lengthY, 2.0);

  } while (lengthSqr < availLengthSqr / 4.0);

  return (config);
}

KenBurnsEffect::KenBurnsConfig KenBurnsEffect::createKBconfigPredefine(
  RGBPlane& plane, uint32 pictureWidth, uint32 pictureHeight,
  uint32 frameWidth, uint32 frameHeight, uint32 sequenceLength,
  uint32 blindLength, uint32 predefine)
{

  KenBurnsConfig config;

  config.origPlane = plane;
  config.outputWidth = frameWidth;
  config.outputHeight = frameHeight;
  config.sequenceLength = sequenceLength;
  config.blindLength = blindLength;

  float maxZoomfactor;
  if ((pictureWidth * 1.0) / (frameWidth * 1.0) * (frameHeight * 1.0)
      < (pictureHeight * 1.0))
    maxZoomfactor = (pictureWidth * 1.0) / (frameWidth * 1.0);
  else
    maxZoomfactor = (pictureHeight * 1.0) / (frameHeight * 1.0);

  if (predefine < 5) {
    config.zoomStart = maxZoomfactor * 0.9;//maxZoomfactor*0.75+rand()*(maxZoomfactor*0.25)/(RAND_MAX*1.0);
    config.zoomEnd = maxZoomfactor * 0.9;//maxZoomfactor*0.75+rand()*(maxZoomfactor*0.25)/(RAND_MAX*1.0);
  } else {
    if (predefine < 9) {
      config.zoomStart = maxZoomfactor * 0.8;//maxZoomfactor*0.75+rand()*(maxZoomfactor*0.25)/(RAND_MAX*1.0);
      config.zoomEnd = maxZoomfactor * 0.9;//maxZoomfactor*0.75+rand()*(maxZoomfactor*0.25)/(RAND_MAX*1.0);
      predefine -= 4;
    } else {
      if (predefine < 13) {
        config.zoomStart = maxZoomfactor * 0.9;//maxZoomfactor*0.75+rand()*(maxZoomfactor*0.25)/(RAND_MAX*1.0);
        config.zoomEnd = maxZoomfactor * 0.8;//maxZoomfactor*0.75+rand()*(maxZoomfactor*0.25)/(RAND_MAX*1.0);
        predefine -= 8;
      } else {
        logger.error() << "Predefine No. <" << predefine
                       << "> not available\n";
        exit(-1);
      }
    }
  }

#ifdef DEBUG
  logger.error()<< "Zooming (" << maxZoomfactor << ") from factor "
                << config.zoomStart << " to " << config.zoomEnd << std::endl;
#endif

  float availableXStart = pictureWidth - frameWidth * config.zoomStart;
  float availableYStart = pictureHeight - frameHeight * config.zoomStart;

  float availableXEnd = pictureWidth - frameWidth * config.zoomEnd;
  float availableYEnd = pictureHeight - frameHeight * config.zoomEnd;

  if ((availableXStart < 0) || (availableYStart < 0) || (availableXEnd < 0)
      || (availableYEnd < 0)) {
    logger.error() << "KenBurnsSequence: picture to small\n";
    // was machen bei einem Fehler?
  }

  float availLengthSqr(powf((availableXStart - availableXEnd), 2.0) + powf(
                         (availableYStart - availableYEnd), 2.0));
  float lengthSqr;

  //  do {

  switch (predefine) {
  case 1: {
    config.startpointX = 0;
    config.startpointY = 0;

    config.endpointX = availableXEnd;
    config.endpointY = availableYEnd;
    break;
  }
  case 2: {
    config.startpointX = availableXStart;
    config.startpointY = 0;

    config.endpointX = 0;
    config.endpointY = availableYEnd;
    break;
  }
  case 3: {
    config.startpointX = availableXStart;
    config.startpointY = availableYStart;

    config.endpointX = 0;
    config.endpointY = 0;
    break;
  }
  case 4: {
    config.startpointX = 0;
    config.startpointY = availableYStart;

    config.endpointX = availableXEnd;
    config.endpointY = 0;
    break;
  }
  }

  // calculate walklength
  float lengthX = fabs(config.startpointX - config.endpointX);
  float lengthY = fabs(config.startpointY - config.endpointY);

  lengthSqr = powf(lengthX, 2.0) + powf(lengthY, 2.0);

  //  } while (false); //lengthSqr < availLengthSqr/4.0);

  return (config);
}
