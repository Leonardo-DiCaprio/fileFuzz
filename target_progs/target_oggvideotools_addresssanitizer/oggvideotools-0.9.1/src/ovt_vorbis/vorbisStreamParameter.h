#ifndef VORBISSTREAMPARAMETER_H_
#define VORBISSTREAMPARAMETER_H_

#include "definition.h"
#include "streamParameter.h"

class VorbisStreamParameter : public StreamParameter {
public:

  uint32 channels;
  uint32 samplerate;
  uint32 datarate;
  uint32 datarateMax;
  uint32 datarateMin;
  uint32 datarateWin;

  uint32 block0;
  uint32 block1;

  VorbisStreamParameter();
  virtual ~VorbisStreamParameter();

  virtual bool operator==(const StreamParameter& param);

  virtual std::string toString();

  virtual StreamParameter* clone();

};

#endif /*VORBISSTREAMPARAMETER_H_*/
