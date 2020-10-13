#include <iostream>
#include <cstring>

#include "theoraExtractor.h"

#include "oggHeader.h"
#include "theoraHeader.h"
#include "theoraStreamParameter.h"
#include "log.h"

TheoraExtractor::TheoraExtractor()
{
}

TheoraExtractor::~TheoraExtractor()
{
}

bool TheoraExtractor::_extract(uint8* data, ExtractorInformation& info)
{

  StreamType*   streaminfo   = (StreamType*) (data);
  TheoraHeader* theoraHeader = (TheoraHeader*) (data + sizeof(StreamType));

  /* if this is not a theora header, return with an error */
  if ((streaminfo->headerType != 0x80) ||
      (strncmp(streaminfo->typeName, "theora", 6) != 0)) {
    logger.error() << "TheoraPosInterpreter::initialize: This page is not a theora bos\n";
    return(false);
  }

  // first extract the parameters
  std::shared_ptr<TheoraStreamParameter> param = std::make_shared<TheoraStreamParameter>();

  // for all the calculation, we need to convert some fields
  theoraHeader->un.pleaseconvert = convert16(theoraHeader->un.pleaseconvert);

  param->framerateNum     = convert32(theoraHeader->frn);
  param->framerateDenom   = convert32(theoraHeader->frd);

  param->pictureX         = convert24(theoraHeader->picw);
  param->pictureY         = convert24(theoraHeader->pich);

  param->aspectRatioNum   = convert24(theoraHeader->parn);
  param->aspectRatioDenom = convert24(theoraHeader->parn);

  param->frameX           = convert16(theoraHeader->fmbw)*16;
  param->frameY           = convert16(theoraHeader->fmbh)*16;

  param->frameXOffset     = theoraHeader->picx;
  param->frameYOffset     = theoraHeader->picy;

  param->videoQuality     = theoraHeader->un.lenbo.qual;
  param->videoBitrate     = convert24(theoraHeader->nombr);

  param->keyframeShift    = theoraHeader->un.lenbo.kfgshift;

  param->colorspace       = (TheoraStreamParameter::ColorSpace) theoraHeader->cs;
  param->pixel_fmt        = (TheoraStreamParameter::PixFormat) theoraHeader->un.lenbo.pf;

  // to have the original packet, we recover the data
  theoraHeader->un.pleaseconvert = convert16(theoraHeader->un.pleaseconvert);

  info.parameter = param;

  /* set the ogg type and the number of header packets */
  info.type = OggType::theora;
  info.numOfHeaderPackets = 3; // the first three packets are headers

  return(true);
}

bool TheoraExtractor::extract(OggPage& oggPage, ExtractorInformation& information)
{
  /* if this is not a Begin Of Stream page, return immediately */
  if (!oggPage->isBOS()) {
    logger.error() << "TheoraPosInterpreter::extract: This page is not a BOS (Begin Of Stream) page\n";
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

bool TheoraExtractor::extract(OggPacket& packet, ExtractorInformation& information)
{
  // if this is not a Begin Of Stream page, return immediately
  if (!packet->isBOS()) {
    logger.error() << "TheoraPosInterpreter::extract: This packet is not a BOS (Begin Of Stream) page\n";
    return(false);
  }

  return _extract(packet->data(), information);

}
