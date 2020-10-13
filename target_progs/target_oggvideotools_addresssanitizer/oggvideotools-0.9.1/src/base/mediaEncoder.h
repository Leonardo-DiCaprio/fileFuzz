#ifndef MEDIAENCODER_H_
#define MEDIAENCODER_H_

#include "definition.h"

#include "mediaConverter.h"

class MediaEncoder : public MediaConverter {
  bool   useFixBunches;
  uint32 bunchsize;

public:
  MediaEncoder();
  virtual ~MediaEncoder();

  void setBunchsize(uint32 bunchsize);
  void useVariableBunches();

};

#endif /*MEDIAENCODER_H_*/
