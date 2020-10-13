#include "katePosInterpreter.h"
#include "kateStreamParameter.h"
#include "log.h"

#include <iostream>
#include <typeinfo>


KatePosInterpreter::KatePosInterpreter()
  : granuleShift(0), granulerateNumerator(1), granulerateDenominator(1)
{
}

KatePosInterpreter::~KatePosInterpreter()
{
}

uint32 KatePosInterpreter::getGranulerateNumerator()
{
  return(granulerateNumerator);
}

uint32 KatePosInterpreter::getGranulerateDenominator()
{
  return(granulerateDenominator);
}

uint8 KatePosInterpreter::getGranuleShift()
{
  return(granuleShift);
}

void KatePosInterpreter::extractFramePos(int64 granulePosition, int64& base, int64& offset)
{

  base = granulePosition>>granuleShift;

  uint64 mask(1);
  mask <<=  granuleShift;
  mask  -=  1;
  offset = (granulePosition&mask);
}

void KatePosInterpreter::initialize(StreamParameter* _param)
{
  KateStreamParameter* param = dynamic_cast<KateStreamParameter*>(_param);

  if (!param) {
    logger.error() << "KatePosInterpreter::initialize: parameter not set correctly\n";
    return;
  }

  granuleShift           = param->granuleShift;
  granulerateNumerator   = param->granulerateNum;
  granulerateDenominator = param->granulerateDenom;

  initialized = true;
  return;
}

double KatePosInterpreter::getTime(int64 granulePos)
{
  if (!initialized) {
    logger.error() << "KatePosInterpreter::initialize: The position interpreter is not initialized yet\n";
    return(-2);
  }

  if (granulePos == -1)
    return(-1);

  int64 base;
  int64 offset;

  extractFramePos(granulePos, base, offset);

  double time = (granulerateDenominator*1.0/granulerateNumerator*1.0)*(base+offset);

  return(time);
}

#if 0
KatePosInterpreter& KatePosInterpreter::operator++()
{
  actualGranulePosition+=1;
  return(*this);
}
#endif

#if 0
GranulePosInterpreter& TheoraPosInterpreter::operator+=(GranulePosInterpreter& _otherPosition)
{
  if (typeid(_otherPosition) != typeid(*this)) {
    logger.error() << "GranulePosInterpreter::operator+=: type is not matching\n";
    return(*this);
  }

  TheoraPosInterpreter* otherPosition = static_cast<TheoraPosInterpreter*>(&_otherPosition);

  if ((keyframeShift != otherPosition->keyframeShift) ||
      (framerateNumerator != otherPosition->framerateNumerator) ||
      (framerateDenominator != otherPosition->framerateDenominator)) {
    logger.error() << "GranulePosInterpreter::operator+=: granulePositions does not match in shift value or framerate\n";
    return(*this);
  }

  if ((actualGranulePosition < 0) || (otherPosition->actualGranulePosition < 0)) {
    logger.error() << "GranulePosInterpreter::operator+=: one or both granulePositions are not valid\n";
    return(*this);
  }

  int64 ownPos1;
  int32 ownPos2;

  extractFramePos(actualGranulePosition, ownPos1, ownPos2);

  int64 otherPos1;
  int32 otherPos2;

  extractFramePos(otherPosition->actualGranulePosition, otherPos1, otherPos2);

  ownPos1 += (otherPos1 + otherPos2);

  actualGranulePosition = ((ownPos1<<keyframeShift)|(ownPos2));

  return(*this);
}
#endif

void KatePosInterpreter::setStreamPosition(OggPacket& packet)
{
  int64_t granpos = packet->granulepos();
  if (granpos >= 0) {
    actualGranulePosition = packet->granulepos();
    packet->setGranulepos(actualGranulePosition);
  }
}


GranulePosInterpreter& KatePosInterpreter::operator-=(GranulePosInterpreter& position)
{
  logger.error() << "GranulePosInterpreter& operator-=: not implemented\n";

  return(*this);
}

