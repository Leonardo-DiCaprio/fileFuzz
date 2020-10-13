#ifndef LOG_H
#define LOG_H

#include <iostream>

class FakeStreambuf : public std::streambuf {
public:
  FakeStreambuf() {}
};

class FakeStream : public std::ostream {
public:
  FakeStreambuf buf;

  FakeStream()
    : std::ostream(&buf) {}
};

class OggLog {
public:
  enum Severity {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
  };

  OggLog()
    : currentLevel(LOG_INFO) {}

  void setLevel(Severity newLevel);
  Severity getLevel() const;

  std::ostream & error() const;
  std::ostream & warning() const;
  std::ostream & info() const;
  std::ostream & debug() const;

  std::ostream & getStream(Severity severity) const;
protected:
  Severity currentLevel;
  FakeStream fakeStream;
};

// Global instance
extern OggLog logger;

#endif
