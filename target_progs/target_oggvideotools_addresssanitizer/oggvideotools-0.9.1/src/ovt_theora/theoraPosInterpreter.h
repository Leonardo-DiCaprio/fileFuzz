#ifndef THEORAPOSINTERPRETER_H_
#define THEORAPOSINTERPRETER_H_

#include "definition.h"
#include "granulePosInterpreter.h"

class TheoraPosInterpreter : public GranulePosInterpreter {

protected:
  uint8 keyframeShift;
  uint32 framerateNumerator;
  uint32 framerateDenominator;


public:
  TheoraPosInterpreter();
  virtual ~TheoraPosInterpreter();

  void extractFramePos(int64 granulePosition, int64& keyframePosition,
                       int32& intraframePosition);
  uint32 getFramerateNumerator();
  uint32 getFramerateDenominator();
  uint8  getKeyframeShift();

  virtual void initialize(StreamParameter* parameter);
  virtual double getTime(int64 granulePos);

  TheoraPosInterpreter& operator++();
  void addKeyframe();
  static bool packetIsKeyframe(OggPacket& packet);

  virtual void setStreamPosition(OggPacket& packet);
  virtual GranulePosInterpreter& operator+=(GranulePosInterpreter& position);
  virtual GranulePosInterpreter& operator-=(GranulePosInterpreter& position);

};

#endif /*THEORAPOSINTERPRETER_H_*/
