#include "mediaOutputDecoder.h"

MediaOutputDecoder::MediaOutputDecoder(const uint8 _streamNo)
  : streamID(_streamNo)
{
}

MediaOutputDecoder::~MediaOutputDecoder()
{
}

uint8 MediaOutputDecoder::getStreamNo() const
{
  return(streamID);
}
