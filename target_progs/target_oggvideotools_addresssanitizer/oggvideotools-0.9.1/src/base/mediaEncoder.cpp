#include "mediaEncoder.h"

MediaEncoder::MediaEncoder()
  : useFixBunches(0), bunchsize(false)
{
}

MediaEncoder::~MediaEncoder()
{
}

void MediaEncoder::setBunchsize(uint32 _bunchsize)
{
  bunchsize     = _bunchsize;
  useFixBunches = true;
}

void MediaEncoder::useVariableBunches()
{
  useFixBunches = false;
}
