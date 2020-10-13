#include <iostream>
#include <cstring>

#include "kateExtractor.h"
#include "definition.h"
#include "oggTypes.h"
#include "oggHeader.h"
#include "kateHeader.h"
#include "kateStreamParameter.h"
#include "katePosInterpreter.h"
#include "log.h"

KateExtractor::KateExtractor()
{
}

KateExtractor::~KateExtractor()
{
}

bool KateExtractor::_extract(uint8* data, ExtractorInformation& info)
{

  StreamType*   streaminfo   = (StreamType*) (data);
  KateHeader* kateHeader     = (KateHeader*) (data + sizeof(StreamType));

  /* if this is not a kate header, return with an error */
  if ((streaminfo->headerType != 0x80) ||
      (memcmp(streaminfo->typeName, "kate\0\0\0", 7) != 0)) {
    // TODO: no size of the passed data, the above could overflow (on read)
    logger.error() << "KatePosInterpreter::initialize: This page is not a kate bos\n";
    return(false);
  }

  // first extract the parameters
  std::shared_ptr<KateStreamParameter> param(new KateStreamParameter);

  param->granulerateNum   = (kateHeader->granulerateNumerator);
  param->granulerateDenom = (kateHeader->granulerateDenominator);
  param->granuleShift     = kateHeader->granuleShift;

  param->language = std::string(kateHeader->language, 16);
  param->category = std::string(kateHeader->category, 16);

  /* are there any old info stored, then delete them */
  info.parameter = param;

  /* set the ogg type and the number of header packets */
  info.type = OggType::kate;
  info.numOfHeaderPackets = kateHeader->numHeaders;

  return(true);
}

bool KateExtractor::extract(OggPage& oggPage, ExtractorInformation& information)
{
  /* if this is not a Begin Of Stream page, return immediately */
  if (!oggPage->isBOS()) {
    logger.error() << "KatePosInterpreter::extract: This page is not a BOS (Begin Of Stream) page\n";
    return(false);
  }

  uint8_t* dataPtr = &(oggPage->data())[0];

  /* get the information starting points within the raw data */
  OggHeader* oggHeader  = (OggHeader*) dataPtr;
  uint8*     data       = dataPtr + sizeof(OggHeader) + oggHeader->tableSegments;

  if (_extract(data, information) == false)
    return(false);

  information.serialNo = oggHeader->serial;

  return(true);
}

bool KateExtractor::extract(OggPacket& packet, ExtractorInformation& information)
{
  // if this is not a Begin Of Stream page, return immediately
  if (!packet->isBOS()) {
    logger.error() << "TheoraPosInterpreter::extract: This packet is not a BOS (Begin Of Stream) page\n";
    return(false);
  }

  if (_extract(packet->data(), information) == false)
    return(false);

  return(true);
}
