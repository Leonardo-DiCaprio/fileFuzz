#ifndef KATEEXTRACTOR_H_
#define KATEEXTRACTOR_H_

#include "streamExtractor.h"

class KateExtractor : public StreamExtractor {
public:
  KateExtractor();
  virtual ~KateExtractor();

  bool _extract(uint8* data, ExtractorInformation& information);

  virtual bool extract(OggPage& page, ExtractorInformation& information);
  virtual bool extract(OggPacket& packet, ExtractorInformation& information);

};

#endif /*KATEEXTRACTOR_H_*/
