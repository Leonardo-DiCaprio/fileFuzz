#ifndef VORBISEXTRACTOR_H_
#define VORBISEXTRACTOR_H_

#include "streamExtractor.h"

class VorbisExtractor : public StreamExtractor {
public:
  VorbisExtractor();
  virtual ~VorbisExtractor();

  bool _extract(uint8* data, ExtractorInformation& info);

  virtual bool extract(OggPage& page, ExtractorInformation& information);
  virtual bool extract(OggPacket& packet, ExtractorInformation& information);

};

#endif /*VORBISEXTRACTOR_H_*/
