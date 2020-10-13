#include <iostream>
#include <cstring>

#include "vorbisExtractor.h"
#include "vorbisStreamParameter.h"
#include "oggHeader.h"
#include "vorbisHeader.h"
#include "log.h"

VorbisExtractor::VorbisExtractor()
{
}

VorbisExtractor::~VorbisExtractor()
{
}

bool VorbisExtractor::_extract(uint8* data, ExtractorInformation& info)
{

  StreamType*   streaminfo   = (StreamType*) (data);
  VorbisHeader* vorbisHeader = (VorbisHeader*) (data + sizeof(StreamType));

  /* if this is not a vorbis header, return with an error */
  if ((streaminfo->headerType != 0x01) ||
      (strncmp(streaminfo->typeName, "vorbis", 6) != 0)) {
    logger.error() << "VorbisExtractor::_extract: This page is not a vorbis bos\n";
    return(false);
  }

  // first extract the parameters
  std::shared_ptr<VorbisStreamParameter> param = std::make_shared<VorbisStreamParameter>();

  param->channels    = vorbisHeader->audioChannels;
  param->samplerate  = vorbisHeader->sampleRate;
  param->datarate    = vorbisHeader->bitrateNom;
  param->datarateMin = vorbisHeader->bitrateMin;
  param->datarateMax = vorbisHeader->bitrateMax;

  param->block0      = 1<<vorbisHeader->blocksize0;
  param->block1      = 1<<vorbisHeader->blocksize1;

  info.parameter = param;

  /* set the ogg type and the number of header packets */
  info.type = OggType::vorbis;
  info.numOfHeaderPackets = 3; // the first three packets are headers

  return(true);
}

bool VorbisExtractor::extract(OggPage& oggPage, ExtractorInformation& information)
{
  /* if this is not a Begin Of Stream page, return immediately */
  if (!oggPage->isBOS()) {
    logger.error() << "VorbisPosInterpreter::extract: This page is not a BOS (Begin Of Stream) page\n";
    return(false);
  }

  uint8_t* dataPtr = &(oggPage->data())[0];
  /* get the information starting points within the raw data */
  OggHeader* oggHeader  = (OggHeader*) dataPtr;
  uint8*     data       = dataPtr + sizeof(OggHeader) + oggHeader->tableSegments;

  if (!_extract(data, information))
    return(false);

  information.serialNo = oggHeader->serial;

  return(true);
}

bool VorbisExtractor::extract(OggPacket& packet, ExtractorInformation& information)
{
  /// if this is not a Begin Of Stream page, return immediately
  if (!packet->isBOS()) {
    logger.error() << "VorbisPosInterpreter::extract: This page is not a BOS (Begin Of Stream) page\n";
    return(false);
  }

  return _extract(packet->data(), information);

}
