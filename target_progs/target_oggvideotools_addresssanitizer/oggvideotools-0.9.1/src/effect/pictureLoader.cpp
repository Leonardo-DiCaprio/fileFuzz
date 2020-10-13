//
// C++ Implementation: pictureLoader
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "pictureLoader.h"

#include <iostream>
#include <cstring>
#include <errno.h>
#include "exception.h"
#include "log.h"

#define SCALEBITS 8
#define ONE_HALF  (1 << (SCALEBITS - 1))
#define FIX(x)    ((int) ((x) * (1L<<SCALEBITS) + 0.5))
#define CLAMP255(x) ((unsigned char)((((x)<0)-1)&((x)|-((x)>255))))

PictureLoader::PictureLoader()
{
}


PictureLoader::~PictureLoader()
{
}

#ifdef WITH_GD2LIB
bool PictureLoader::load(RGBPlane& retPlane, const std::string& filename, uint32 _width, uint32 _height,
                         bool useBiggest)
{
  SuffixType type = identifySuffix(filename);
  if (type == suffix_unknown) {
    logger.error() << "PictureLoader::load: Cannot identify suffix of <"<<filename<<">\n";
    return(false);
  }

  gdImagePtr im(0);

  FILE* in;
  in = fopen(filename.c_str(), "rb");

  if (in == 0) {
    logger.error() << "PictureLoader::load: Cannot open file <"<<filename<<">: "
                   << strerror(errno) << "\n";
    return(false);
  }

  if (type == suffix_jpg) {
    im = gdImageCreateFromJpeg(in);
  } else if (type == suffix_png) {
    im = gdImageCreateFromPng(in);
  } else if (type == suffix_gif) {
    im = gdImageCreateFromGif(in);
  }

  fclose(in);

  if (im == 0) {
    logger.error() << "PictureLoader::load: Error reading image file <"<<filename<<">\n";
    return(false);
  }

  if ((_width != 0) && (_height != 0) && ( _width != gdImageSX(im) ) &&
      (_height != gdImageSY(im))) {

    uint32 origWidth(gdImageSX(im));
    uint32 origHeight(gdImageSY(im));

    /* calculate the new size -> picture must fit into the given rectangle */
    float factorX = (_width*1.0)/(origWidth*1.0);
    float factorY = (_height*1.0)/(origHeight*1.0);
    float factor(1.0);

#ifdef DEBUG
    logger.debug() << "wanted: "<<_width<<"x"<<_height<<"  orig: "
                   <<origWidth<<"x"<<origHeight<<std::endl;
#endif

    if (useBiggest) {
      if (factorX < factorY)
        factor = factorY;
      else
        factor = factorX;
    } else {
      if (factorX < factorY)
        factor = factorX;
      else
        factor = factorY;
    }
#ifdef DEBUG
    logger.debug() << "recalculating ("<<factor<<") image to "
                   <<(uint32) (origWidth*factor+0.5)<< "x"
                   << (uint32) (origHeight*factor+0.5)<<std::endl;
#endif

    gdImagePtr resampled = gdImageCreateTrueColor((uint32) (origWidth
                           *factor+0.5), (uint32) (origHeight*factor+0.5));

    if (!resampled) {
      throw OggException("PictureLoader::load: failed to allocate image buffer\n");
    }

    gdImageCopyResampled(resampled, im, 0, 0, 0, 0, resampled->sx,
                         resampled->sy, origWidth, origHeight);

    retPlane = convertToRgbPlane(resampled);

    gdImageDestroy(resampled);
  } else {
    retPlane = convertToRgbPlane(im);
  }

  gdImageDestroy(im);

  return (true);
}

RGBPlane PictureLoader::convertToRgbPlane(gdImagePtr im)
{

  uint32 width = gdImageSX(im);
  uint32 height = gdImageSY(im);

  RGBPlane pic(width, height);

  int c(0);
  uint32 x(0);

  for (uint32 i(0); i<height; ++i)
    for (uint32 j(0); j<width; ++j) {
      c = gdImageGetPixel(im, j, i);
      pic->plane[x++] = gdImageRed(im, c);
      pic->plane[x++] = gdImageGreen(im, c);
      pic->plane[x++] = gdImageBlue(im,c);
      pic->plane[x++] = gdImageAlpha(im,c);
    }

  return(pic);
}

PictureLoader::SuffixType PictureLoader::identifySuffix(const std::string& filename)
{
  std::string::size_type suffixStart(filename.find_last_of('.'));

  if (suffixStart == std::string::npos) {
    return (suffix_unknown);
  }

  std::string suffix(filename.substr(suffixStart+1));

  if ((suffix == "jpg") || (suffix == "JPG") || (suffix == "jpeg") || (suffix
      == "JPEG")) {
    return (suffix_jpg);
  }

  if ((suffix == "png") || (suffix == "PNG")) {
    return (suffix_png);
  }

  if ((suffix == "gif") || (suffix == "GIF")) {
    return (suffix_gif);
  }

  return (suffix_unknown);

}

bool PictureLoader::save(RGBPlane& pic, const std::string& filename, uint32 newWidth,
                         uint32 newHeight)
{

  int actColor;
  int planeCount(0);

  SuffixType type = identifySuffix(filename);
  if (type == suffix_unknown) {
    logger.error() << "PictureLoader::identifySuffix: Cannot identify suffix of <"
                   << filename << ">\n";
    return(false);
  }

  gdImagePtr im = gdImageCreateTrueColor(pic->width, pic->height);

  for (uint32 i(0); i < pic->height; ++i)
    for (uint32 j(0); j < pic->width; ++j) {
      int red = pic->plane[planeCount++];
      int green = pic->plane[planeCount++];
      int blue = pic->plane[planeCount++];
      actColor = gdImageColorAllocate(im, red, green, blue);

      planeCount++; // alpha channel not in use

      gdImageSetPixel(im, j, i, actColor);

    }

  FILE* out = fopen(filename.c_str(), "wb");
  if (out == 0) {
    logger.error() << "PictureLoader::save: Cannot open file <"<<filename<<">: "
                   << strerror(errno) << "\n";
    return(false);
  }

  if ((newWidth != 0) || (newHeight != 0)) {

    if (newWidth == 0)
      newWidth = pic->width*newHeight/pic->height;

    if (newHeight == 0)
      newHeight = pic->height*newWidth/pic->width;

    gdImagePtr resampled;
    resampled = gdImageCreateTrueColor(newWidth, newHeight);
    if (!resampled) {
      throw OggException("PictureLoader::save: failed to allocate image buffer\n");
    }

    gdImageCopyResampled(resampled, im, 0, 0, 0, 0, resampled->sx,
                         resampled->sy, pic->width, pic->height);

    switch (type) {

    case suffix_jpg:
      gdImageJpeg(resampled, out, -1);
      break;

    case suffix_png:
      gdImagePng(resampled, out);
      break;

      //    case suffix_gif:

    default:
      logger.error() << "cannot identify suffix\n";

    }
    /* Write JPEG using default quality */
    gdImageDestroy(resampled);

  } else {

    switch (type) {

    case suffix_jpg:
      gdImageJpeg(im, out, -1);
      break;

    case suffix_png:
      gdImagePng(im, out);
      break;

      //    case suffix_gif:

    default:
      logger.error() << "cannot identify suffix\n";

    }

  }

  /* Close file */
  if (fclose(out) != 0) {
    logger.error() << "Error writing file <" << filename << ">: "
                   << strerror(errno) << "\n";
    gdImageDestroy(im);
    return(false);
  }

  /* Destroy the image */
  gdImageDestroy(im);

  return (true);
}
#endif //WITH_GD2LIB

#ifdef HAVE_LIBTHEORAENC

void PictureLoader::exportYCrCb_theora(RGBPlane& picture, th_ycbcr_buffer& buffer, int pixel_format )
{

  uint32 frameWidth;
  uint32 frameHeight;
  uint32 XOffset;
  uint32 YOffset;

  /* recalculate the buffer (must be multiple of 16) */
  frameWidth = (picture->width+15)&~0xF;
  frameHeight = (picture->height+15)&~0xF;

  // We force the offset to be even.
  // This ensures that the chroma samples align properly with the luma
  // samples.

  XOffset = ((frameWidth - picture->width)/4); //&~1;
  YOffset = ((frameHeight - picture->height)/4); //&~1;

//    logger.debug() << width <<" x "<<height<<"      "<<frameWidth<<" x "<<frameHeight <<" "<<XOffset<<" "<<YOffset<<std::endl;

  uint32 stride = frameWidth;

  if ((frameWidth  != (uint32)buffer[0].width) ||
      (frameHeight != (uint32)buffer[0].height)) {

    /* delete old planes */
    delete buffer[0].data;
    delete buffer[1].data;
    delete buffer[2].data;

    /* create a new YCbCrPlane */
    buffer[0].width = frameWidth;
    buffer[0].height = frameHeight;
    buffer[0].stride = stride;
    buffer[0].data = new uint8[frameWidth*frameHeight];
//        memset(buffer[0].data, 0x00, frameWidth*frameHeight);

    buffer[1].width = frameWidth/2;
    buffer[1].height = frameHeight/2;
    buffer[1].stride = stride/2;
    buffer[1].data = new uint8[frameWidth*frameHeight/4];
//        memset(buffer[1].data, 0x00, frameWidth*frameHeight/4);

    buffer[2].width = frameWidth/2;
    buffer[2].height = frameHeight/2;
    buffer[2].stride = stride/2;
    buffer[2].data = new uint8[frameWidth*frameHeight/4];
//        memset(buffer[2].data, 0x00, frameWidth*frameHeight/4);

  }


  int wrap, wrap3;

  wrap = stride;
  wrap3 = picture->width * 4;

  uint32 HeightPrecalculation0x;
  uint32 HeightPrecalculation1x;
  uint32 CromaPrecalculation;

  uint32 position00;
  uint32 position01;
  uint32 position10;
  uint32 position11;

  uint32 inPos00;
  uint32 inPos01;
  uint32 inPos10;
  uint32 inPos11;

  uint32 red_sample;
  uint32 green_sample;
  uint32 blue_sample;

  uint32 cromaPos;

  for (uint32 i(0); i<(uint32)(picture->height+1)/2; ++i) {

    HeightPrecalculation0x = (2*(i+YOffset))*buffer[0].stride;
    HeightPrecalculation1x = (2*(i+YOffset)+1)*buffer[0].stride;
    CromaPrecalculation = (i+YOffset)*buffer[1].stride;

    for (uint32 j(0); j<(uint32)(picture->width+1)/2; ++j) {

      position00 = HeightPrecalculation0x+(2*(j+XOffset));
      position01 = HeightPrecalculation0x+(2*(j+XOffset)+1);
      position10 = HeightPrecalculation1x+(2*(j+XOffset));
      position11 = HeightPrecalculation1x+(2*(j+XOffset)+1);

      inPos00 = 4*((2*i)*picture->width+(2*j));
      inPos01 = 4*((2*i)*picture->width+(2*j+1));
      inPos10 = 4*((2*i+1)*picture->width+(2*j));
      inPos11 = 4*((2*i+1)*picture->width+(2*j+1));

      cromaPos = CromaPrecalculation+(j+XOffset);


      buffer[0].data[position00] = (FIX(0.29900) * picture->plane[inPos00]
                                    + FIX(0.58700) * picture->plane[inPos00+1]
                                    + FIX(0.11400) * picture->plane[inPos00+2]
                                    + ONE_HALF) >> SCALEBITS;

      buffer[0].data[position01] = (FIX(0.29900) * picture->plane[inPos01]
                                    + FIX(0.58700) * picture->plane[inPos01+1]
                                    + FIX(0.11400) * picture->plane[inPos01+2]
                                    + ONE_HALF) >> SCALEBITS;

      buffer[0].data[position10] = (FIX(0.29900) * picture->plane[inPos10]
                                    + FIX(0.58700) * picture->plane[inPos10+1]
                                    + FIX(0.11400) * picture->plane[inPos10+2]
                                    + ONE_HALF) >> SCALEBITS;

      buffer[0].data[position11] = (FIX(0.29900) * picture->plane[inPos11]
                                    + FIX(0.58700) * picture->plane[inPos11+1]
                                    + FIX(0.11400) * picture->plane[inPos11+2]
                                    + ONE_HALF) >> SCALEBITS;

      red_sample = picture->plane[inPos00] + picture->plane[inPos01] + picture->plane[inPos10] + picture->plane[inPos11];

      green_sample = picture->plane[inPos00+1] + picture->plane[inPos01+1] + picture->plane[inPos10+1] + picture->plane[inPos11+1];

      blue_sample = picture->plane[inPos00+2] + picture->plane[inPos01+2] + picture->plane[inPos10+2] + picture->plane[inPos11+2];

      buffer[1].data[cromaPos] =  ((-FIX(0.16874) * red_sample - FIX(0.33126) * green_sample +FIX(0.50000) * blue_sample + 4 * ONE_HALF- 1) >> (SCALEBITS + 2)) + 128;

      buffer[2].data[cromaPos] =  ((FIX(0.50000) * red_sample - FIX(0.41869) * green_sample -FIX(0.08131) * blue_sample + 4 * ONE_HALF - 1) >> (SCALEBITS + 2)) + 128;


    }
  }
}


void PictureLoader::exportYCrCb_444_theora(RGBPlane& picture, th_ycbcr_buffer& buffer)
{

  uint32 frameWidth;
  uint32 frameHeight;
  uint32 XOffset;
  uint32 YOffset;

  /* recalculate the buffer (must be multiple of 16) */
  frameWidth = (picture->width+15)&~0xF;
  frameHeight = (picture->height+15)&~0xF;

  // We force the offset to be even.
  // This ensures that the chroma samples align properly with the luma
  // samples.

  XOffset = ((frameWidth - picture->width)/2)&~1;
  YOffset = ((frameHeight - picture->height)/2)&~1;

//    logger.debug() << width <<" x "<<height<<"      "<<frameWidth<<" x "<<frameHeight <<" "<<XOffset<<" "<<YOffset<<std::endl;

  uint32 stride = frameWidth;

  if ((frameWidth  != (uint32)buffer[0].width) ||
      (frameHeight != (uint32)buffer[0].height)) {

    /* delete old planes */
    delete buffer[0].data;
    delete buffer[1].data;
    delete buffer[2].data;

    /* create a new YCbCrPlane */
    buffer[0].width = frameWidth;
    buffer[0].height = frameHeight;
    buffer[0].stride = stride;
    buffer[0].data = new uint8[frameWidth*frameHeight];

    buffer[1].width = frameWidth;
    buffer[1].height = frameHeight;
    buffer[1].stride = stride;
    buffer[1].data = new uint8[frameWidth*frameHeight];

    buffer[2].width = frameWidth;
    buffer[2].height = frameHeight;
    buffer[2].stride = stride;
    buffer[2].data = new uint8[frameWidth*frameHeight];

  }

  uint32 HeightPrecalculation;
  uint32 ycrcbPosition;
  uint32 rgbPosition;

  uint8 red_sample;
  uint8 green_sample;
  uint8 blue_sample;

  uint32 cromaPos;

  for (uint32 i(0); i<(uint32)picture->height; ++i) {

    HeightPrecalculation = (i+YOffset)*buffer[0].stride;

    for (uint32 j(0); j<(uint32)picture->width; ++j) {

      ycrcbPosition = HeightPrecalculation+(j+XOffset);

      rgbPosition = 4*(i*picture->width+j);

      red_sample = picture->plane[rgbPosition];
      green_sample = picture->plane[rgbPosition+1];
      blue_sample = picture->plane[rgbPosition+2];

      buffer[0].data[ycrcbPosition] = (FIX(0.29900) * red_sample
                                       + FIX(0.58700) * green_sample
                                       + FIX(0.11400) * blue_sample
                                       + ONE_HALF) >> SCALEBITS;
      buffer[0].data[ycrcbPosition] = 0;

      int32 cr = ( -1 * FIX(0.168736) * red_sample
                   - FIX(0.331264) * green_sample
                   + FIX(0.5) * blue_sample
                   + ONE_HALF ) >> SCALEBITS ;
      static uint32 cn(0);
      //cr =
      if (i < picture->height/4 )
        buffer[1].data[ycrcbPosition] = cn++;//128 + cr;
      else
        buffer[1].data[ycrcbPosition] = 0;//128 + cr;
      // logger.debug() << (uint32) buffer[1].data[ycrcbPosition] << " ";

      int32 cb =  (FIX(0.5) * red_sample
                   - FIX(0.418688) * green_sample
                   - FIX(0.081312) * blue_sample
                   + ONE_HALF) >> SCALEBITS;
      buffer[2].data[ycrcbPosition] = 0; //128 +cb;

    }
  }
  //abort();
}


RGBPlane PictureLoader::importYCrCb_theora(const th_ycbcr_buffer& buffer, uint32 _width, uint32 _height, uint32 XOffset, uint32 YOffset, int pixel_format)
{

  uint32 width;
  uint32 height;

  // what size to use?
  if ((_width == 0) || (_height == 0)) {
    width = buffer[0].width;
    height = buffer[0].height;
    XOffset = 0;
    YOffset = 0;
  } else {
    width = _width;
    height = _height;
  }

  RGBPlane retPlane(width, height);

  /* Theora spec 4.4.1/4.4.2/4.4.3:
     4:2:0 is subsampled on X and Y, 4:2:2 on X, and 4:4:4 is not subsampled. */
  unsigned int CbCr_subshift_x = (pixel_format==TH_PF_444)?0:1;
  unsigned int CbCr_subshift_y = (pixel_format==TH_PF_420)?1:0;

  uint8* out     = retPlane->plane;
  for (int row=YOffset; row<height+YOffset; row++) {
    for (int col=XOffset; col<width+XOffset; col++) {
      int Y = buffer[0].data[row*buffer[0].stride+col];
      /* Theora spec 4.4.4:
         The sampling locations are defined relative to the frame, not the picture region.*/
      int CrCb_pos = (row>>CbCr_subshift_y)*buffer[1].stride+(col>>CbCr_subshift_x);
      int Cb = buffer[1].data[CrCb_pos];
      int Cr = buffer[2].data[CrCb_pos];
      /* Theora spec 4.3.1/4.3.2:
         Y,Cb,Cr have offsets 16, 128, and 128 respectively.*/
      /* This can be made marginally faster by performing all
         computation over a common power-of-two denominator to conserve the
         multiplication on Y and replace the division with a shift.
         Although somewhat faster it doesn't make it fast compared to a SIMD implementation
         so this instead favors accuracy.
         This can also be made faster on some platforms for using a table to
         replace multiplication, but that isn't likely to be helpful if only
         a single smallish frame is being written.
         The two (minor) speedups mentioned here are implemented in libtheora's
         player_example.c. An inaccurate but fairly fast SIMD implementation
         can be found in liboggplay. */
      int r=(1904000*Y+2609823*Cr-363703744)/1635200;
      *(out++) = CLAMP255(r);
      int g=(3827562*Y-1287801*Cb-2672387*Cr+447306710)/3287200;
      *(out++) = CLAMP255(g);
      int b=(952000*Y+1649289*Cb-225932192)/817600;
      *(out++) = CLAMP255(b);
      *(out++) = 255;
    }
  }

  return(retPlane);
}

#endif


