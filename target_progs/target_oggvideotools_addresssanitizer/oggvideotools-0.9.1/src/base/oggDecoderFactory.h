#ifndef OGGDECODERFACTORY_H_
#define OGGDECODERFACTORY_H_

#include "oggPage.h"
#include "oggPacket.h"
#include "oggTypes.h"
#include "oggStreamDecoder.h"

class OggDecoderFactory {
public:
  OggDecoderFactory();
  virtual ~OggDecoderFactory();

  static OggStreamDecoder* getOggStreamDecoder(OggPage& page);
  static OggStreamDecoder* getOggStreamDecoder(OggPacket& packet);

  static OggType getStreamType(OggPage& page);
  static OggType getStreamType(OggPacket& packet);
};

#endif /*OGGDECODERFACTORY_H_*/
