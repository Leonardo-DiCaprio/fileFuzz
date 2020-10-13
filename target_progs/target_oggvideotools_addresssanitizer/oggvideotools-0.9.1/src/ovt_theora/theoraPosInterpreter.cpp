#include "theoraPosInterpreter.h"
#include "theoraStreamParameter.h"

#include <iostream>
#include <typeinfo>
#include "log.h"


TheoraPosInterpreter::TheoraPosInterpreter()
  : keyframeShift(6), framerateNumerator(1), framerateDenominator(1)
{
}

TheoraPosInterpreter::~TheoraPosInterpreter()
{
}

uint32 TheoraPosInterpreter::getFramerateNumerator()
{
  return(framerateNumerator);
}

uint32 TheoraPosInterpreter::getFramerateDenominator()
{
  return(framerateDenominator);
}

uint8 TheoraPosInterpreter::getKeyframeShift()
{
  return(keyframeShift);
}

void TheoraPosInterpreter::extractFramePos(int64 granulePosition, int64& keyframePosition, int32& intraframePosition)
{

  keyframePosition = granulePosition>>keyframeShift;

  uint64 mask(1);
  mask <<=  keyframeShift;
  mask  -=  1;
  intraframePosition = (granulePosition&mask);

}

void TheoraPosInterpreter::initialize(StreamParameter* _param)
{
  TheoraStreamParameter* param = dynamic_cast<TheoraStreamParameter*>(_param);

  if (!param) {
    logger.error() << "TheoraPosInterpreter::initialize: parameter not set correctly\n";
    return;
  }

  keyframeShift        = param->keyframeShift;
  framerateNumerator   = param->framerateNum;
  framerateDenominator = param->framerateDenom;

  initialized = true;
  return;
}

/*
void TheoraPosInterpreter::initialize(OggPage oggPage)
{
  if (!oggPage.isBOS()) {
    logger.error() << "TheoraPosInterpreter::initialize: This page is not a BOS (Begin Of Stream) page\n";
    return;
  }

  OggHeader*    oggHeader         = (OggHeader*) (oggPage.data());
  StreamType*   streamInformation = (StreamType*) (oggPage.data() + sizeof(OggHeader) + oggHeader->tableSegments);
  TheoraHeader* theoraHeader      = (TheoraHeader*) (oggPage.data() + sizeof(OggHeader) + oggHeader->tableSegments + sizeof(StreamType));

  if ((streamInformation->headerType != 0x80) ||
      (strncmp(streamInformation->typeName, "theora", 6) != 0)) {
    logger.error() << "TheoraPosInterpreter::initialize: This page is not a theora bos\n";
    return;
  }

  // for all the calculation, we need to convert some fields
  theoraHeader->un.pleaseconvert = convert16(theoraHeader->un.pleaseconvert);

  framerateNumerator   = convert32(theoraHeader->frn);
  framerateDenominator = convert32(theoraHeader->frd);

  keyframeShift        = theoraHeader->un.lenbo.kfgshift;

  // to have the original packet, we recover the data
  theoraHeader->un.pleaseconvert = convert16(theoraHeader->un.pleaseconvert);

  initialized = true;
}

void TheoraPosInterpreter::initialize(OggPacket m_oggPacket)
{

  StreamType*   streamInformation = (StreamType*) (m_oggPacket.data());
  TheoraHeader* theoraHeader      = (TheoraHeader*) (m_oggPacket.data() + sizeof(StreamType));

  if ((streamInformation->headerType != 0x80) ||
      (strncmp(streamInformation->typeName, "theora", 6) != 0)) {
    logger.error() << "TheoraPosInterpreter::initialize: This page is not a theora bos\n";
    return;
  }

  // for all the calculation, we need to convert some fields
  theoraHeader->un.pleaseconvert = convert16(theoraHeader->un.pleaseconvert);

  framerateNumerator   = convert32(theoraHeader->frn);
  framerateDenominator = convert32(theoraHeader->frd);

  keyframeShift        = theoraHeader->un.lenbo.kfgshift;

  // to have the original packet, we recover the data
  theoraHeader->un.pleaseconvert = convert16(theoraHeader->un.pleaseconvert);

  initialized = true;
}

*/

double TheoraPosInterpreter::getTime(int64 granulePos)
{
  if (!initialized) {
    logger.error() << "TheoraPosInterpreter::initialize: The position interpreter is not initialized yet\n";
    return(-2);
  }

  if (granulePos == -1)
    return(-1);

  int64 pos1;
  int32 pos2;

  extractFramePos(granulePos, pos1, pos2);

  double time = (framerateDenominator*1.0/framerateNumerator*1.0)*(pos1+pos2);

//  logger.debug() << "time extractor: "<<granulePos<<" ("<<pos1<<":"<<pos2<<")    -> "<<time<<"s \n";

  return(time);
}

void TheoraPosInterpreter::addKeyframe()
{
  /* The first keyframe is 0 */
//  if (actualGranulePosition == 0) {
//    actualGranulePosition = 1<<keyframeShift;
//    return;
//  }

  int64 pos1;
  int32 pos2;

  extractFramePos(actualGranulePosition, pos1, pos2);

  pos1 += (pos2 + 1);

  actualGranulePosition = pos1<<keyframeShift;

}

TheoraPosInterpreter& TheoraPosInterpreter::operator++()
{
  actualGranulePosition+=1;
  return(*this);
}

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

bool TheoraPosInterpreter::packetIsKeyframe(OggPacket& packet)
{
  // is there any data available
  if (packet->length()<=0)
    return(false);

  if (packet->data()[0]&0x80) {
    return(false);
  }

  if (!(packet->data()[0]&0x40)) {
    return(true);
  }

  return(false);
}

void TheoraPosInterpreter::setStreamPosition(OggPacket& packet)
{
  /* is this a keyframe */
  if (packetIsKeyframe(packet))
    addKeyframe();
  else
    actualGranulePosition+=1;

  packet->setGranulepos(actualGranulePosition);
}


GranulePosInterpreter& TheoraPosInterpreter::operator-=(GranulePosInterpreter& position)
{
  logger.error() << "GranulePosInterpreter& operator-=: not implemented\n";

  return(*this);
}
