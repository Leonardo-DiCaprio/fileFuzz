#ifndef VORBISPOSINTERPRETER_H_
#define VORBISPOSINTERPRETER_H_

#include "granulePosInterpreter.h"

class VorbisPosInterpreter : public GranulePosInterpreter {

protected:

  enum LastBlock {
    none,
    block0,
    block1
  };

  uint32    samplerate;
  uint32    blocksize0;
  uint32    blocksize1;
//    uint8     channels;

  LastBlock lastBlock;

public:
  VorbisPosInterpreter();
  virtual ~VorbisPosInterpreter();

  virtual void initialize(StreamParameter* parameter);
  /*
      virtual void initialize(OggPage firstPage);
      virtual void initialize(OggPacket firstPacket);
  */
  virtual double getTime(int64 granulePos);

  void addBlock0();
  void addBlock1();

  virtual void setStreamPosition(OggPacket& packet);

  virtual GranulePosInterpreter& operator+=(GranulePosInterpreter& position);
  virtual GranulePosInterpreter& operator-=(GranulePosInterpreter& position);

};

#endif /*VORBISPOSINTERPRETER_H_*/
