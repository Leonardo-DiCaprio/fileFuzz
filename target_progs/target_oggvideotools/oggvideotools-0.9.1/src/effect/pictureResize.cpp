//
// C++ Implementation: pictureResize
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "pictureResize.h"

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "exception.h"
#include "log.h"

#define INIT_CLIP int32 tmp
#define CLIP(x,n)   tmp = (int32)(x+0.5); \
                if (tmp > 255) n=255; \
          else if (tmp < 0) n=0; \
          else n = (uint8)(tmp);

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)<(y))?(y):(x))

PictureResize::PictureResize()
{
}

PictureResize::~PictureResize()
{
}

uint32 PictureResize::calculateKernelValue(RGBPlane& pic, float posX,
    float posY, float radius, bool p)
{

  radius = sqrt(radius);

  uint32 xStart(MAX(0,posX-radius+0.5));
  uint32 xEnd(MIN(posX+radius+0.5,(float)pic->width));

  uint32 yStart(MAX(0,posY-radius+0.5));
  uint32 yEnd(MIN(posY+radius+0.5,(float)pic->height));

  float weightsCounter(0);

  float valueRed(0);
  float valueGreen(0);
  float valueBlue(0);

  float tmpWeight;
  float tmpDistance;
  uint32 position;
  uint32 positionHeight;

#ifdef DEBUG
  uint32 counter1(0);
  uint32 overall(0);

  if (p) {
    logger.debug() << "kernel calculation at position "<<posX<<" x "<<posY
                   <<" rad "<<radius <<" kerne "<<xStart<<"x"<<yStart<<"->"<<xEnd
                   <<"x"<<yEnd<<" \n";
  }
#endif

  for (uint32 i(yStart); i<yEnd; ++i) {

    positionHeight = i*pic->width;

    for (uint32 j(xStart); j<xEnd; ++j) {

      tmpDistance = (((float)i)-posY)*(((float)i)-posY) + (((float)j)
                    -posX)*(((float)j)-posX);
      tmpDistance = sqrt(tmpDistance);
      tmpWeight = getWeight(tmpDistance, radius); // getWeight(sqrt(tmpDistance), radius);
#ifdef DEBUG
      overall++;
#endif
      if (tmpWeight <= 0) {
#ifdef DEBUG
        if (p) {
          logger.debug() << "     Pos: "<<j<<" x "<<i<<" -> distance "
                         <<tmpDistance <<"radius: "<<radius<<" weight: "
                         <<tmpWeight << " - UNCOUNTED   ";
        }
        counter1++;
#endif
        continue;
      }

      //static uint32 cnt(0);
      //  if ((cnt++ % 10) == 0)
      //logger.debug() << ".";

      position = 4*(positionHeight+j);
#ifdef DEBUG
      if (p) {
        logger.debug() << "     Pos: "<<j<<" x "<<i<<" -> distance "
                       <<tmpDistance <<"radius: "<<radius<<" weight: "
                       <<tmpWeight;
      }

      if (position > 4*pic->width*pic->height) {
        logger.error() << "Error: calculating for Position "<<posX<<" x "
                       <<posY<<" at kernel position "<<j<<" x "<<i
                       <<"  with radius "<<radius<<" \n";
        logger.error() << "Picture size: "<<pic->width << " x "
                       << pic->height<<"    Kernel window: "<<xStart <<":"
                       <<yStart<<" -> "<<xEnd<<":"<<yEnd<<"\n";
        abort();
      }
#endif
      valueRed += pic->plane[position]*tmpWeight;
      valueGreen += pic->plane[position+1]*tmpWeight;
      valueBlue += pic->plane[position+2]*tmpWeight;
      weightsCounter += tmpWeight;
    }
  }
#ifdef DEBUG
  if (p)
    logger.debug() << "        Unused: "<<counter1<<"/"<<overall<<"\n";
#endif
  uint32 retValue(0);

  uint8* RGB = (uint8*)(&retValue);

  INIT_CLIP;

  CLIP((valueRed/weightsCounter), RGB[0])
  ;
  CLIP((valueGreen/weightsCounter), RGB[1])
  ;
  CLIP((valueBlue/weightsCounter), RGB[2])
  ;

  return (retValue);
}

uint32 PictureResize::calculateKernelValueFix(RGBPlane& pic, float posX,
    float posY, float radius, bool p)
{

  radius = sqrt(radius);

  uint32 xStart(MAX(0,posX-radius+0.5));
  uint32 xEnd(MIN(posX+radius+0.5,(float)pic->width));

  uint32 yStart(MAX(0,posY-radius+0.5));
  uint32 yEnd(MIN(posY+radius+0.5,(float)pic->height));

  uint32 radiusF(radius+0.51);

  uint32 weightsCounter(0);

  uint32 valueRed(0);
  uint32 valueGreen(0);
  uint32 valueBlue(0);

  int32 tmpWeight;
  uint32 tmpDistance;
  uint32 position;
  uint32 positionHeight;

  uint32 posXF = (uint32)(posX+0.5);
  uint32 posYF = (uint32)(posY+0.5);

  uint32 factor(1);
  uint32 factorCounter(0);
  uint32 size(((xEnd-xStart)*(yEnd-yStart)));

  if (size > 128) {
    factor = size / 32;
  }

#ifdef DEBUG
  uint32 counter1(0);
  uint32 overall(0);

  if (p) {
    logger.debug() << "kernel calculation at position "<<posXF<<" x "<<posYF
                   <<" rad "<<radiusF <<" kerne "<<xStart<<"x"<<yStart<<"->"<<xEnd
                   <<"x"<<yEnd<<" \n";
  }
#endif

  struct pos {
    uint32 x;
    uint32 y;
  };

  uint32 i;
  uint32 j;

  for (uint32 r(0); r<2*radiusF; ++r) {

    j = xStart + rand()%(xEnd - xStart);
    i = yStart + rand()%(yEnd - yStart);

    positionHeight = i*pic->width;

    tmpDistance = (i-posYF)*(i-posYF) + (j-posXF)*(j-posXF);
    tmpDistance = (uint32)(sqrt(tmpDistance)+0.5);
    tmpWeight = getWeightFix(tmpDistance, radiusF);

#ifdef DEBUG
    overall++;
#endif
    if (tmpWeight <= 0) {
#ifdef DEBUG
      if (p) {
        logger.debug() << "     Pos: "<<j<<" x "<<i<<" -> distance "
                       <<tmpDistance <<" radius: "<<radiusF<<" weight: "
                       <<tmpWeight << " - UNCOUNTED   ";
      }
      counter1++;
#endif
      continue;
    }

#ifdef DEBUG
    if (p) {
      logger.debug() << "     Pos: "<<j<<" x "<<i<<" -> distance "
                     <<tmpDistance <<" radius: "<<radiusF<<" weight: "
                     <<tmpWeight;
    }
#endif

    position = 4*(positionHeight+j);

#ifdef DEBUG
    if (position > 4*pic->width*pic->height) {
      logger.debug() << "Error: calculating for Position "<<posX<<" x "<<posY
                     <<" at kernel position "<<j<<" x "<<i<<"  with radius "
                     <<radiusF<<" \n";
      logger.debug() << "Picture size: "<<pic->width << " x "<< pic->height
                     <<"    Kernel window: "<<xStart <<":"<<yStart<<" -> "<<xEnd
                     <<":"<<yEnd<<"\n";
      abort();
    }
#endif

    valueRed += pic->plane[position]*tmpWeight;
    valueGreen += pic->plane[position+1]*tmpWeight;
    valueBlue += pic->plane[position+2]*tmpWeight;
    weightsCounter += tmpWeight;
  }

#ifdef DEBUG
  if (p)
    logger.debug() << "        Unused: "<<counter1<<"/"<<overall<<"\n";
#endif

  uint32 retValue(0);

  uint8* RGB = (uint8*)(&retValue);

  INIT_CLIP;

  if (weightsCounter != 0) {
    CLIP((valueRed/weightsCounter), RGB[0])
    ;
    CLIP((valueGreen/weightsCounter), RGB[1])
    ;
    CLIP((valueBlue/weightsCounter), RGB[2])
    ;
  } else {
    RGB[0] = pic->plane[4*(posYF*pic->width+posXF)];
    RGB[1] = pic->plane[4*(posYF*pic->width+posXF)+1];
    RGB[2] = pic->plane[4*(posYF*pic->width+posXF)+2];
  }

  return (retValue);
}

float PictureResize::getWeight(float distance, float radius)
{
  // should be a sinc

  /*  -> lets save time on the expence of security
   * if ((radius <= 0) || (distance > radius))
   *    return(0);
   */
  return (1.0 - distance/radius);

  //    return(1.0 - sqrt(distance)/sqrt(radius));
}

int32 PictureResize::getWeightFix(uint32 distance, uint32 radius)
{
  // should be a sinc

  /*  -> lets save time on the expence of security
   * if ((radius <= 0) || (distance > radius))
   *    return(0);
   */
  return (1000 - distance*1000/radius);
}

RGBPlane PictureResize::kernelLowpass(RGBPlane& picture, float radius)
{

  RGBPlane retPlane(picture->width, picture->height);

  float kernelRadius((1.0-radius)*picture->height/4.0);

  kernelRadius *= kernelRadius;

  if (kernelRadius < 0.708) // sqrt(0.5) this is the lease radius size, a picture can be
    kernelRadius = 0.708;

  uint32 heightAddition;

  for (uint32 i(0); i<retPlane->height; ++i) {

    heightAddition = 4*i*picture->width;

    for (uint32 j(0); j<retPlane->width; ++j) {
      uint32* _plane = (uint32*)(&retPlane->plane[heightAddition + 4*j]);
      (*_plane) = calculateKernelValueFix(picture, j, i, kernelRadius);
    }
  }

  //    logger.debug() << "  DONE ! \n";

  return (retPlane);

}

uint32 PictureResize::linearInterpolation(RGBPlane pic, float x, float y)
{
  uint32 pixelDistance = 4;
  uint32 pos_x1 = (int)(x);
  uint32 pos_x2 = (int)(x+1.0);
  uint32 pos_y1 = (int)(y);
  uint32 pos_y2 = (int)(y+1.0);

  if (pos_x2 >= pic->width)
    pos_x2 = pic->width-1;

  if (pos_y2 >= pic->height)
    pos_y2 = pic->height-1;

  float part_x = (float)(x - pos_x1);
  float part_y = (float)(y - pos_y1);

  float value_x1y1;
  float value_x1y2;
  float value_x2y1;
  float value_x2y2;

  float inter_x1y1_x1y2;
  float inter_x2y1_x2y2;
  float endpoint;

  uint32 retValue(0);

  uint8* RGB = (uint8*)(&retValue);

  /* red */

  value_x1y1 = pic->plane[pixelDistance*(pos_y1*pic->width+pos_x1)];
  value_x1y2 = pic->plane[pixelDistance*(pos_y2*pic->width+pos_x1)];
  value_x2y1 = pic->plane[pixelDistance*(pos_y1*pic->width+pos_x2)];
  value_x2y2 = pic->plane[pixelDistance*(pos_y2*pic->width+pos_x2)];

  inter_x1y1_x1y2 = (value_x1y2-value_x1y1)*part_y + value_x1y1;
  inter_x2y1_x2y2 = (value_x2y2-value_x2y1)*part_y + value_x2y1;

  endpoint = (inter_x2y1_x2y2 - inter_x1y1_x1y2) * part_x + inter_x1y1_x1y2
             + 0.5;

  if (endpoint > 255)
    endpoint = 255;

  if (endpoint < 0)
    endpoint = 0;

  RGB[0] = (uint8)endpoint;

  /* green */

  value_x1y1 = pic->plane[pixelDistance*(pos_y1*pic->width+pos_x1)+1];
  value_x1y2 = pic->plane[pixelDistance*(pos_y2*pic->width+pos_x1)+1];
  value_x2y1 = pic->plane[pixelDistance*(pos_y1*pic->width+pos_x2)+1];
  value_x2y2 = pic->plane[pixelDistance*(pos_y2*pic->width+pos_x2)+1];

  inter_x1y1_x1y2 = (value_x1y2-value_x1y1)*part_y + value_x1y1;
  inter_x2y1_x2y2 = (value_x2y2-value_x2y1)*part_y + value_x2y1;

  endpoint = (inter_x2y1_x2y2 - inter_x1y1_x1y2) * part_x + inter_x1y1_x1y2
             + 0.5;

  if (endpoint > 255)
    endpoint = 255;

  if (endpoint < 0)
    endpoint = 0;

  RGB[1] = (uint8)endpoint;

  /* blue */

  value_x1y1 = pic->plane[pixelDistance*(pos_y1*pic->width+pos_x1)+2];
  value_x1y2 = pic->plane[pixelDistance*(pos_y2*pic->width+pos_x1)+2];
  value_x2y1 = pic->plane[pixelDistance*(pos_y1*pic->width+pos_x2)+2];
  value_x2y2 = pic->plane[pixelDistance*(pos_y2*pic->width+pos_x2)+2];

  inter_x1y1_x1y2 = (value_x1y2-value_x1y1)*part_y + value_x1y1;
  inter_x2y1_x2y2 = (value_x2y2-value_x2y1)*part_y + value_x2y1;

  endpoint = (inter_x2y1_x2y2 - inter_x1y1_x1y2) * part_x + inter_x1y1_x1y2
             + 0.5;

  if (endpoint > 255)
    endpoint = 255;

  if (endpoint < 0)
    endpoint = 0;

  RGB[2] = (uint8)endpoint;

  /* ALPHA */

  value_x1y1 = pic->plane[pixelDistance*(pos_y1*pic->width+pos_x1)+3];
  value_x1y2 = pic->plane[pixelDistance*(pos_y2*pic->width+pos_x1)+3];
  value_x2y1 = pic->plane[pixelDistance*(pos_y1*pic->width+pos_x2)+3];
  value_x2y2 = pic->plane[pixelDistance*(pos_y2*pic->width+pos_x2)+3];

  inter_x1y1_x1y2 = (value_x1y2-value_x1y1)*part_y + value_x1y1;
  inter_x2y1_x2y2 = (value_x2y2-value_x2y1)*part_y + value_x2y1;

  endpoint = (inter_x2y1_x2y2 - inter_x1y1_x1y2) * part_x + inter_x1y1_x1y2
             + 0.5;

  if (endpoint > 255)
    endpoint = 255;

  if (endpoint < 0)
    endpoint = 0;

  RGB[3] = (uint8)endpoint;

  return (retValue);

}

RGBPlane PictureResize::resize(RGBPlane& picture, uint32 width, uint32 height,
                               uint8 quality)
{

  RGBPlane retPlane(width, height);

  float resizeFactorX(((float)picture->width)/((float)retPlane->width));
  float resizeFactorY(((float)picture->height)/((float)retPlane->height));
  float radius((resizeFactorX*resizeFactorX + resizeFactorY*resizeFactorY)
               /(0.5*quality));
  uint32 heightAddition;

  if (radius < 0.708)
    radius = 0.708;

#ifdef DEBUG
  logger.debug() << "Resizing from "<<picture->width<<" : "<<picture->height
                 <<"  to  "<<retPlane->width<<" : "<<retPlane->height<<"\n";
  logger.debug() << "using resizefactor "<<resizeFactorX<<" : "<<resizeFactorY
                 <<"  with radius "<<radius<<"\n";
#endif
  bool p(false);

  if ((resizeFactorX < 0.6) || (resizeFactorX > 1.6)) {
    logger.debug() << "\nKernel\n";
    for (uint32 i(0); i< retPlane->height; ++i) {

      heightAddition = i*retPlane->width;

      for (uint32 j(0); j < retPlane->width; ++j) {
#ifdef DEBUG
        if ((j==100) && (i==100))
          p=true;
        else
          p=false;
#endif

        ((uint32*)(retPlane->plane))[j+heightAddition]
          = calculateKernelValue(picture, j *resizeFactorX, i
                                 *resizeFactorY, radius, p);
      }
    }
  } else {
    logger.debug() << "\nLinear\n";
    for (uint32 i(0); i< retPlane->height; ++i) {

      heightAddition = i*retPlane->width;

      for (uint32 j(0); j < retPlane->width; ++j) {

#ifdef DEBUG
        if ((j==100) && (i==100))
          p=true;
        else
          p=false;
#endif

        ((uint32*)(retPlane->plane))[j+heightAddition]
          = linearInterpolation(picture, j *resizeFactorX, i
                                *resizeFactorY);
      }
    }
  }

  return (retPlane);

}

RGBPlane PictureResize::resize(RGBPlane& picture, float resizeFactorX, float resizeFactorY, uint8 quality)
{

  RGBPlane retPlane(picture->width*resizeFactorX, picture->height*resizeFactorY);

  float radius((resizeFactorX*resizeFactorX + resizeFactorY*resizeFactorX)/(0.5*quality));
  uint32 heightAddition;

  if (radius < 1.2) // sqrt(0.5) this is the lease radius size, a picture can be
    radius = 1.2;

//  if (radius < 0.708) // sqrt(0.5) this is the lease radius size, a picture can be
//    radius = 0.708;

  bool p(false);

  logger.debug() << "\n\nKernel\n\n";

  for (uint32 i(0); i< retPlane->height; ++i) {

    heightAddition = i*retPlane->width;

    for (uint32 j(0); j < retPlane->width; ++j) {
      if ((i==100) && (j==100))
        p=true;
      else
        p=false;

      ((uint32*)(retPlane->plane))[j+heightAddition] = calculateKernelValue(picture, ((float)j)
          *resizeFactorX, ((float)i)*resizeFactorY, radius);
    }
  }

  return (retPlane);

}

RGBPlane PictureResize::resize(RGBPlane& picture, float resizeFactor,
                               uint8 quality)
{
  return (resize(picture, resizeFactor, resizeFactor, quality));
}

RGBPlane PictureResize::reframe(RGBPlane & picture, uint32 width,
                                uint32 height, uint8 quality, uint32 background, double aspectCorrection)
{
  if ((picture->width == width) && (picture->height == height))
    return (picture);

  if (((picture->width == width) || (picture->height == height)) && aspectCorrection > 0.999 && aspectCorrection < 1.0001 )
    return reframe_fixed(picture, width, height, background);

  RGBPlane newPlane(width, height);

  uint32 planesize(width*height);

  // fill the plane with the given background
  uint32* plPtr((uint32*)(newPlane->plane));
  for (uint32 i(0); i<planesize; ++i)
    plPtr[i] = background;
  // setBackground

  uint32 offsetY(0);
  uint32 offsetX(0);
  float resizeFactor(1);

  if (((float)(picture->height*newPlane->width)/((float)picture->width*aspectCorrection))
      < ((float)newPlane->height)) {
    // we work with a height offset
    offsetY = (uint32) ((((float)newPlane->height)
                         - (((float)(picture->height * newPlane->width))
                            /((float)picture->width*aspectCorrection)))/2.0+0.5);
    offsetX = 0;
    resizeFactor = (((float)picture->width*aspectCorrection/(float)newPlane->width)); //((float)newPlane->width)/((float)width);
  } else {
    // we work with a width offset
    offsetY = 0;
    offsetX = (uint32) ((((float)newPlane->width)
                         - (((float)(picture->width*aspectCorrection *newPlane->height))
                            /((float)picture->height)))/2.0+0.5);
    resizeFactor = (((float)picture->height/(float)newPlane->height)); //((float)newPlane->height)/((float)picture->height);
  }

#ifdef DEBUG
  logger.debug() << "Reframe - OffsetX: "<<offsetX<<" OffsetY: "<<offsetY
                 <<" resize : "<<resizeFactor<<std::endl;
#endif

  uint32 position_new;
  float resizePlaneCounterX(0);
  float resizePlaneCounterY(0);
  float radius((resizeFactor*resizeFactor)/(0.25*quality));
  //    float radius(resizeFactor/2.0);
//  if (radius < 1.2) // sqrt(0.5) this is the lease radius size, a picture can be
//    radius = 1.2;
  if (radius < 1) // sqrt(0.5) this is the lease radius size, a picture can be
    radius = 1;

  // place the picture into the new frame

  for (uint32 i(offsetY); i<(newPlane->height-offsetY); ++i) {
    for (uint32 j(offsetX); j <(newPlane->width-offsetX); ++j) {
      position_new = (((float)i)*newPlane->width+j);
      ((uint32*)(newPlane->plane))[position_new]
        = calculateKernelValue(picture, resizePlaneCounterX,
                               resizePlaneCounterY, radius);
      resizePlaneCounterX += (resizeFactor/aspectCorrection);
    }
    resizePlaneCounterY += resizeFactor;
    resizePlaneCounterX = 0;
  }

  return (newPlane);
}

RGBPlane PictureResize::reframe_fixed(RGBPlane & picture, uint32 width,
                                      uint32 height, uint32 background)
{
  std::cout << "fixed reframe (from " << picture->width <<":"<<picture->height<<" to " << width << ":"<<height<<")  \n";
  if (picture->width > width || picture->height > height)
    throw OggException("picture too big");

  if ((picture->width == width) && (picture->height == height))
    return (picture);

  RGBPlane newPlane(width, height);

  uint32 planesize(width*height);

  // fill the plane with the given background
  uint32* plPtr((uint32*)(newPlane->plane));
  for (uint32 i(0); i<planesize; ++i)
    plPtr[i] = background;
  // setBackground

  uint32* plPtrOrig((uint32*)(picture->plane));
  uint32 offsetX((width - picture->width)/2);
  uint32 offsetY((height - picture->height)/2);

  std::cout << "offset: X=" << offsetX << "  offset Y=" << offsetY << std::endl;

  for (uint32 i(0); i<picture->height; ++i) {
    uint heightAddNew = (i+offsetY)*width;
    uint heightAddOrig = i*picture->width;
    for (uint32 j(0); j < picture->width; ++j) {
      plPtr[heightAddNew+offsetX+j] = plPtrOrig[heightAddOrig+j];
    }
  }

  return (newPlane);
}

RGBPlane PictureResize::subframe(RGBPlane & picture, uint32 newWidth,
                                 uint32 newHeight, float offsetWidth, float offsetHeight,
                                 float scaleFactor, uint8 quality)
{

  if (((((float)newWidth)/scaleFactor)+offsetWidth> picture->width)
      || ((((float)newHeight) /scaleFactor)+offsetHeight> picture->height)) {
    logger.error() << "new width: "<<newWidth<<" / "<<scaleFactor<<" + "
                   <<offsetWidth << " = " << (((float)newWidth)/scaleFactor)
                   +offsetWidth <<" must be smaller then "<<picture->width
                   <<std::endl;
    logger.error() << "new width: "<<newHeight<<" / "<< scaleFactor<<" + "
                   <<offsetHeight << " = " <<(((float)newHeight)/scaleFactor)
                   +offsetHeight <<" must be smaller then "<<picture->height
                   <<std::endl;
    throw OggException("PicConverter::subPic: new width/height is/are to big");
  }

  RGBPlane retPlane = RGBPlane(newWidth, newHeight);

  float resizeFactor(1.0/scaleFactor);
  float radius((resizeFactor*resizeFactor)/(0.5*quality));
  uint32 heightAddition;

  if (radius < 0.708) // sqrt(0.5) this is the lease radius size, a picture can be
    radius = 0.708;

  //    logger.debug() << "subframe: resize factor (j*resize) "<<resizeFactor<<"   radius: "<<radius<<"\n";
  if ((resizeFactor < 0.6) || (resizeFactor > 1.6)) {

    for (uint32 i(0); i < newHeight; ++i) {
      heightAddition = i*newWidth;
      for (uint32 j(0); j < newWidth; ++j) {
        ((uint32*)(retPlane->plane))[j+heightAddition]
          = calculateKernelValue(picture, ((float)j)
                                 *resizeFactor+offsetWidth, ((float)i)
                                 *resizeFactor+offsetHeight, radius);

      }
    }
  } else {
    for (uint32 i(0); i < newHeight; ++i) {
      heightAddition = i*newWidth;
      for (uint32 j(0); j < newWidth; ++j) {
        ((uint32*)(retPlane->plane))[j+heightAddition]
          = linearInterpolation(picture, ((float)j) *resizeFactor
                                +offsetWidth, ((float)i)*resizeFactor
                                +offsetHeight);

      }
    }

  }

  return (retPlane);

}

RGBPlane PictureResize::concatenate(RGBPlane& picture1, RGBPlane& picture2, RGBPlane& picture3)
{
  if (picture1.getHeight() != picture2.getHeight()) {

    logger.error()
        << picture1.getHeight() <<" " <<picture2.getHeight();
    throw(OggException("Height of picture 1 and 2 do not match"));
  }

  if (picture1.getWidth() != picture2.getWidth()) {
    throw(OggException("Width of picture 1 and 2 do not match"));
  }

  RGBPlane retPlane(picture1.getWidth()*2, picture1.getHeight(), 0xffffffff);

  uint32 width = picture1.getWidth();
  for (uint32 hcnt(0); hcnt<picture1.getHeight(); ++hcnt) {
    uint32* retRgbaPlane = ((uint32*)retPlane->plane)+width*hcnt*2;
    uint32* pic1RgbaPlane = ((uint32*)picture1->plane)+width*hcnt;
    uint32* pic2RgbaPlane = ((uint32*)picture2->plane)+width*hcnt;
    memcpy(retRgbaPlane, pic1RgbaPlane, width*sizeof(uint32));
    memcpy(retRgbaPlane+width, pic2RgbaPlane, width*sizeof(uint32));
  }

  return retPlane;
}

