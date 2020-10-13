#ifndef THEORACONFIG_H_
#define THEORACONFIG_H_

#include "definition.h"

#include "mediaConfig.h"

class TheoraConfig : public MediaConfig {
public:
  TheoraConfig();
  virtual ~TheoraConfig();

  uint32 pictureX;
  uint32 pictureY;
  uint32 frameX;
  uint32 frameY;
  uint32 frameXOffset;
  uint32 frameYOffset;

  uint32 aspectRatioNum;
  uint32 aspectRatioDedum;

  uint32 framerateNum;
  uint32 framerateDenum;

  uint32 videoQuality;
  uint32 videoBitrate;

};

#endif /*THEORACONFIG_H_*/
