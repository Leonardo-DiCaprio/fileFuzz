#include "vorbisPosInterpreter.h"

#include <iostream>
#include <typeinfo>

#include "vorbisStreamParameter.h"
#include "log.h"

struct VorbisPackHeader {
  char dataType:1;
  char block:1;
  char blsht:6;
};

VorbisPosInterpreter::VorbisPosInterpreter()
  : samplerate(0), blocksize0(0), blocksize1(0), lastBlock(none)
{
}

VorbisPosInterpreter::~VorbisPosInterpreter()
{
}

void VorbisPosInterpreter::initialize(StreamParameter* _param)
{
  VorbisStreamParameter* param = dynamic_cast<VorbisStreamParameter*>(_param);

  if (!param) {
    logger.error() << "VorbisPosInterpreter::initialize: parameter not set correctly\n";
    return;
  }

  samplerate = param->samplerate;
  blocksize0 = param->block0;
  blocksize1 = param->block1;

  initialized = true;

  return;
}

/*
void VorbisPosInterpreter::initialize(OggPage oggPage)
{

  OggHeader*    oggHeader         = (OggHeader*) (oggPage.data());
  StreamType*   streamInformation = (StreamType*) (oggPage.data() + sizeof(OggHeader) + oggHeader->tableSegments);
  VorbisHeader* vorbisHeader      = (VorbisHeader*) (oggPage.data() + sizeof(OggHeader) + + oggHeader->tableSegments + sizeof(StreamType));

  if ((streamInformation->headerType != 0x01) ||
      (strncmp(streamInformation->typeName, "vorbis", 6) != 0)) {
    logger.error() << "VorbisPosInterpreter::initialize: this page is not a vorbis bos\n";
    return;
  }

  samplerate = vorbisHeader->sampleRate;
  channels   = vorbisHeader->audioChannels;

  blocksize0 = 1<<vorbisHeader->blocksize0;
  blocksize1 = 1<<vorbisHeader->blocksize1;

}

void VorbisPosInterpreter::initialize(OggPacket m_oggPacket)
{
  if (m_oggPacket.length() < (sizeof(StreamType) + sizeof(VorbisHeader))) {
    logger.error() << "VorbisPosInterpreter::initialize: Error: page is to small - cannot parse header\n";
    return;
  }

  StreamType*   streamInformation = (StreamType*) (m_oggPacket.data());
  VorbisHeader* vorbisHeader      = (VorbisHeader*) (m_oggPacket.data() + sizeof(StreamType));

  if ((streamInformation->headerType != 0x01) ||
      (strncmp(streamInformation->typeName, "vorbis", 6) != 0)) {
    logger.error() << "VorbisPosInterpreter::initialize: this page is not a vorbis bos\n";
    return;
  }


  samplerate = vorbisHeader->sampleRate;
  channels   = vorbisHeader->audioChannels;

  blocksize0 = 1<<vorbisHeader->blocksize0;
  blocksize1 = 1<<vorbisHeader->blocksize1;

}
*/

double VorbisPosInterpreter::getTime(int64 granulePos)
{
  double time = (granulePos*1.0)/(samplerate*1.0);

  return(time);
}

void VorbisPosInterpreter::addBlock0()
{
  switch (lastBlock) {
  case block0:
    actualGranulePosition += blocksize0/2;
    break;
  case block1:
    actualGranulePosition += (blocksize1/4 + blocksize0/4);
    break;
  default:
    /* nothing to be done */
    break;
  }

  lastBlock = block0;
}

void VorbisPosInterpreter::addBlock1()
{
  switch (lastBlock) {
  case block0:
    actualGranulePosition += (blocksize1/4 + blocksize0/4);
    break;
  case block1:
    actualGranulePosition += blocksize1/2;
    break;
  default:
    /* nothing to be done */
    break;
  }

  lastBlock = block1;
}

GranulePosInterpreter& VorbisPosInterpreter::operator+=(GranulePosInterpreter& _otherPosition)
{
  if (typeid(_otherPosition) != typeid(*this)) {
    logger.error() << "GranulePosInterpreter::operator+=: type is not matching\n";
    return(*this);
  }

  VorbisPosInterpreter* otherPosition = static_cast<VorbisPosInterpreter*>(&_otherPosition);

  if (samplerate != otherPosition->samplerate) {
    logger.error() << "VorbisPosInterpreter::operator+=: granulePositions does not match in samplerate or channel numbers\n";
    return(*this);
  }

  if ((blocksize0 != otherPosition->blocksize0) || (blocksize1 != otherPosition->blocksize1)) {
    logger.error() << "VorbisPosInterpreter::operator+=: granulePositions does not match in the blocksizes\n";
    return(*this);
  }

  actualGranulePosition += otherPosition->actualGranulePosition;

  return(*this);
}

GranulePosInterpreter& VorbisPosInterpreter::operator-=(GranulePosInterpreter& _otherPosition)
{
  if (typeid(_otherPosition) != typeid(*this)) {
    logger.error() << "GranulePosInterpreter::operator+=: type is not matching\n";
    return(*this);
  }

  VorbisPosInterpreter* otherPosition = static_cast<VorbisPosInterpreter*>(&_otherPosition);

  if (samplerate != otherPosition->samplerate) {
    logger.error() << "VorbisPosInterpreter::operator+=: granulePositions does not match in samplerate or channel numbers\n";
    return(*this);
  }

  if ((blocksize0 != otherPosition->blocksize0) || (blocksize1 != otherPosition->blocksize1)) {
    logger.error() << "VorbisPosInterpreter::operator+=: granulePositions does not match in the blocksizes\n";
    return(*this);
  }

  actualGranulePosition -= otherPosition->actualGranulePosition;

  return(*this);
}

void VorbisPosInterpreter::setStreamPosition(OggPacket& packet)
{
//  packet.setGranulepos(getPosition());

  VorbisPackHeader* packHead = (VorbisPackHeader*)(packet->data());

  if (packHead->block)
    addBlock1();
  else
    addBlock0();

//  logger.debug() << "Granule Position: "<<packet.granulepos();
  packet->setGranulepos(getPosition());
//  logger.debug() << "-> "<<packet.granulepos()<<std::endl;

}
