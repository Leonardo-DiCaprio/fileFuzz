#ifndef KATEPOSINTERPRETER_H_
#define KATEPOSINTERPRETER_H_

#include "definition.h"
#include "granulePosInterpreter.h"

class KatePosInterpreter : public GranulePosInterpreter {

protected:
  uint8 granuleShift;
  uint32 granulerateNumerator;
  uint32 granulerateDenominator;

  void extractFramePos(int64 granulePosition, int64& base, int64& offset);

public:
  KatePosInterpreter();
  virtual ~KatePosInterpreter();

  uint32 getGranulerateNumerator();
  uint32 getGranulerateDenominator();
  uint8  getGranuleShift();

  virtual void initialize(StreamParameter* parameter);
  virtual double getTime(int64 granulePos);

  //KatePosInterpreter& operator++();

  virtual void setStreamPosition(OggPacket& packet);
  //virtual GranulePosInterpreter& operator+=(GranulePosInterpreter& position);
  virtual GranulePosInterpreter& operator-=(GranulePosInterpreter& position);

};

#endif /*KATEPOSINTERPRETER_H_*/
