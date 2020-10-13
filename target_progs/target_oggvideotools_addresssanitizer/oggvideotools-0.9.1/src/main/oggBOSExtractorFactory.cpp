#include <string.h>
#include "oggBOSExtractorFactory.h"
#include "theoraExtractor.h"
#include "vorbisExtractor.h"
#include "kateExtractor.h"
#include "theoraPosInterpreter.h"
#include "vorbisPosInterpreter.h"
#include "katePosInterpreter.h"
//#include "theoraDecoder.h"
//#include "vorbisEncoder.h"

OggBOSExtractorFactory::OggBOSExtractorFactory()
{
}

OggBOSExtractorFactory::~OggBOSExtractorFactory()
{
}

/*
static MediaDecoder* OggBOSExtractorFactory::createDecoder(OggPage& page)
{

}

static MediaDecoder* OggBOSExtractorFactory::createEncoder(OggPacket& packet)
{

}
*/

bool OggBOSExtractorFactory::extractInformation(OggPage& page, ExtractorInformation& information)
{
  switch (getStreamType(page)) {

  case OggType::theora: {
    TheoraExtractor extractor;
    return(extractor.extract(page, information));
  }

  case OggType::vorbis: {
    VorbisExtractor extractor;
    return(extractor.extract(page, information));
  }

  case OggType::kate: {
    KateExtractor extractor;
    return(extractor.extract(page, information));
  }

  default:
    break;

  }

  /* could not interpret the bos page */
  return(false);
}

bool OggBOSExtractorFactory::extractInformation(OggPacket& packet, ExtractorInformation& information)
{
  switch (getStreamType(packet)) {

  case OggType::theora: {
    TheoraExtractor extractor;
    return(extractor.extract(packet, information));
  }

  case OggType::vorbis: {
    VorbisExtractor extractor;
    return(extractor.extract(packet, information));
  }

  case OggType::kate: {
    KateExtractor extractor;
    return(extractor.extract(packet, information));
  }

  default:
    break;

  }

  /* could not interpret the bos page */
  return(false);
}

GranulePosInterpreter* OggBOSExtractorFactory::extractPositionInterpreter(ExtractorInformation& info)
{
  GranulePosInterpreter* retPosInterpreter(0);

  switch (info.type) {
  case OggType::theora:
    retPosInterpreter = new TheoraPosInterpreter;
    break;

  case OggType::vorbis:
    retPosInterpreter = new VorbisPosInterpreter;
    break;

  case OggType::kate:
    retPosInterpreter = new KatePosInterpreter;
    break;

  default:
    break;
  }

  if (retPosInterpreter)
    retPosInterpreter->initialize(info.parameter.get());

  return(retPosInterpreter);
}

OggType OggBOSExtractorFactory::getStreamType(OggPage& page)
{
  uint8* type = &(page->data())[0]+page->getHeaderLength();

  uint8 i=1;
  for (; i< to_integral(OggType::maxOggType); ++i) {
    if (memcmp(type, OggTypeMap[i], MAXIDCHARS) == 0)
//    if ((*type) == OggTypeMap[i])
      return ((OggType)i);
  }

  return (OggType::unknown);

}

OggType OggBOSExtractorFactory::getStreamType(OggPacket& packet)
{
  uint8* type = packet->data();

  uint8 i=1;
  for (; i< to_integral(OggType::maxOggType); ++i) {
    if (memcmp(type, OggTypeMap[i], MAXIDCHARS) == 0)
//    if ((*type) == OggTypeMap[i])
      return ((OggType)i);
  }

  return (OggType::unknown);
}
