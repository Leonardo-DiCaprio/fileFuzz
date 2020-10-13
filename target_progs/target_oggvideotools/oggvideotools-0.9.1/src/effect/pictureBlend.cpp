//
// C++ Implementation: pictureBlend
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "pictureBlend.h"
#include "exception.h"

PictureBlend::PictureBlend()
{
}


PictureBlend::~PictureBlend()
{
}

RGBPlane PictureBlend::crossfade(RGBPlane & plane1, RGBPlane & plane2, float velocity)
{

  if ((plane1->width != plane2->width) || (plane1->height != plane2->height))
    throw OggException("can not crossfade, planes not matching");

  uint32 size = plane1->width*plane1->height*4;

  RGBPlane retPlane(plane1->width, plane1->height);

  uint32 pixel1;
  uint32 pixel2;
  uint32 newPixel;

  for (uint32 i(0); i<size; i+=4) {

    /* red */
    pixel1 = plane1->plane[i];
    pixel2 = plane2->plane[i];
    newPixel = (uint32)(pixel1*(1.0-velocity) + pixel2*velocity);

    if (newPixel> 0xFF)
      newPixel = 0xFF;

    retPlane->plane[i] = (uint8)newPixel;

    /* green */
    pixel1 = plane1->plane[i+1];
    pixel2 = plane2->plane[i+1];
    newPixel = (uint32)(pixel1*(1.0-velocity) + pixel2*velocity);

    if (newPixel> 0xFF)
      newPixel = 0xFF;

    retPlane->plane[i+1] = (uint8)newPixel;

    /* blue */
    pixel1 = plane1->plane[i+2];
    pixel2 = plane2->plane[i+2];
    newPixel = (uint32)(pixel1*(1.0-velocity) + pixel2*velocity);

    if (newPixel> 0xFF)
      newPixel = 0xFF;

    retPlane->plane[i+2] = (uint8)newPixel;

  }

  return (retPlane);
}

RGBPlane PictureBlend::alphaBlend(RGBPlane& origPlane, RGBPlane & alphaPlane, float intensity)
{
  float factor;
  uint32 position;

  uint32 pixel1;
  uint32 pixel2;
  uint32 newPixel;

  RGBPlane retPlane(origPlane->width, origPlane->height);

  uint32 positionAlpha;

  for (uint32 j(0); j < origPlane->height; ++j)
    for (uint32 i(0); i < origPlane->width; ++i) {

      // if the alpha plane is smaller than the original plane, just copy the data
      if ((i<alphaPlane->width) && (j<alphaPlane->height)) {
        position = 4*(j*origPlane->width+i);

        positionAlpha = 4*(j*alphaPlane->width+i);

        factor = intensity*((127-alphaPlane->plane[positionAlpha+3])*1.0)/127.0;
        for (uint32 k(0); k<3; ++k) {
          pixel1 = origPlane->plane[position+k];
          pixel2 = alphaPlane->plane[positionAlpha+k];
          newPixel = (uint32)(pixel1 * (1.0-factor) + pixel2 * factor );

          if (newPixel> 0xFF)
            newPixel = 0xFF;

          retPlane->plane[position+k] = (uint8)newPixel;
        }

      } else {
        position = j*origPlane->width+i;
        ((uint32*)(retPlane->plane))[position] = ((uint32*)(origPlane->plane))[position];
      }

    }
  return (retPlane);

}


