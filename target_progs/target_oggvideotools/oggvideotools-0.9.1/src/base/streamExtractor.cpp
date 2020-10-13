#include "streamExtractor.h"

ExtractorInformation::ExtractorInformation()
  : type(OggType::unknown), serialNo(0), parameter(0), numOfHeaderPackets(0)
{
}

ExtractorInformation::ExtractorInformation(const ExtractorInformation& extractorInfo)
  : type(extractorInfo.type), serialNo(extractorInfo.serialNo),
    parameter(0), numOfHeaderPackets(extractorInfo.numOfHeaderPackets)
{
  parameter = extractorInfo.parameter;
}

ExtractorInformation& ExtractorInformation::operator=(const ExtractorInformation& extractorInfo)
{
  type               = extractorInfo.type;
  serialNo           = extractorInfo.serialNo;
  numOfHeaderPackets = extractorInfo.numOfHeaderPackets;

  parameter = extractorInfo.parameter;

  return(*this);
}

ExtractorInformation::~ExtractorInformation()
{
}

StreamExtractor::StreamExtractor()
{
}

StreamExtractor::~StreamExtractor()
{
}

