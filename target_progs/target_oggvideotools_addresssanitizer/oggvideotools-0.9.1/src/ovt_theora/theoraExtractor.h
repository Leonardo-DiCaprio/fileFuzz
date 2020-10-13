#ifndef THEORAEXTRACTOR_H_
#define THEORAEXTRACTOR_H_

#include "streamExtractor.h"

class TheoraExtractor : public StreamExtractor {
public:
  TheoraExtractor();
  virtual ~TheoraExtractor();

  bool _extract(uint8* data, ExtractorInformation& information);

  virtual bool extract(OggPage& page, ExtractorInformation& information);
  virtual bool extract(OggPacket& packet, ExtractorInformation& information);

};

#endif /*THEORAEXTRACTOR_H_*/
