#include "oggDecoderFactory.h"

OggDecoderFactory::OggDecoderFactory()
{
}

OggDecoderFactory::~OggDecoderFactory()
{
}


OggStreamDecoder* OggDecoderFactory::getOggStreamDecoder(OggPage& page)
{
  if (oggPage.bos()) {
    switch (getStreamType(OggPage)) {
    case ogg_theora:
      return()
    }
  } else
    return(0);
}



OggStreamDecoder* OggDecoderFactory::getOggStreamDecoder(OggPacket& packet)
{

}

