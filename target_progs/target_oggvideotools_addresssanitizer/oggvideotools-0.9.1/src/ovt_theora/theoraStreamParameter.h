#ifndef THEORASTREAMPARAMETER_H_
#define THEORASTREAMPARAMETER_H_

#include "definition.h"
#include "streamParameter.h"

class TheoraStreamParameter : public StreamParameter {
public:

  enum PixFormat {
    pf_420,
    pf_rsvd,
    pf_422,
    pf_444
  };

  enum ColorSpace {
    unspecified,
    ITU_470M,
    ITU_470BG
  };

  uint32 pictureX;
  uint32 pictureY;
  uint32 frameX;
  uint32 frameY;
  uint32 frameXOffset;
  uint32 frameYOffset;

  uint32 aspectRatioNum;
  uint32 aspectRatioDenom;

  uint32 framerateNum;
  uint32 framerateDenom;

  uint32 videoQuality;
  uint32 videoBitrate;

  uint8  keyframeShift;

  PixFormat  pixel_fmt;
  ColorSpace colorspace;

  TheoraStreamParameter();
  virtual ~TheoraStreamParameter();

  virtual bool operator==(const StreamParameter& param);

  virtual std::string toString();

  virtual StreamParameter* clone();

  void calculateFrame();

};

#endif /*THEORASTREAMPARAMETER_H_*/
