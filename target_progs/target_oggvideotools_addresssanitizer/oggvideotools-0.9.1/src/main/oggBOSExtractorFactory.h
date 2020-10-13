#ifndef OGGBOSEXTRACTORFACTORY_H_
#define OGGBOSEXTRACTORFACTORY_H_

#include "oggTypes.h"
#include "granulePosInterpreter.h"
//#include "oggMediaStream.h"
#include "streamExtractor.h"

class OggBOSExtractorFactory {

public:
  OggBOSExtractorFactory();
  virtual ~OggBOSExtractorFactory();

  /*
      static MediaDecoder* createStreamDecoder(ExtractorInformation& information);
      static MediaDecoder* createStreamEncoder(ExtractorInformation& information);
  */

  static bool extractInformation(OggPage& page, ExtractorInformation& information);
  static bool extractInformation(OggPacket& packet, ExtractorInformation& information);
  static GranulePosInterpreter* extractPositionInterpreter(ExtractorInformation& information);

  static OggType getStreamType(OggPage& page);
  static OggType getStreamType(OggPacket& packet);
};

#endif /*OGGBOSEXTRACTORFACTORY_H_*/
