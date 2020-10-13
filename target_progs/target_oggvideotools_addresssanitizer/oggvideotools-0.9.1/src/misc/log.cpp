#include "log.h"
#include <iostream>

OggLog logger;

void OggLog::setLevel(Severity newLevel)
{
  currentLevel = newLevel;
}

OggLog::Severity OggLog::getLevel() const
{
  return currentLevel;
}

std::ostream & OggLog::error() const
{
  return getStream(LOG_ERROR);
}

std::ostream & OggLog::warning() const
{
  return getStream(LOG_WARNING);
}

std::ostream & OggLog::info() const
{
  return getStream(LOG_INFO);
}

std::ostream & OggLog::debug() const
{
  return getStream(LOG_DEBUG);
}

std::ostream & OggLog::getStream(Severity severity) const
{
  if (severity >= currentLevel) {
    return std::cerr;
  } else {
    return (std::ostream&)fakeStream;
  }
}

