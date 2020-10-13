#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>

class OggException : public std::runtime_error {
public:
  OggException(const std::string & msg)
    : std::runtime_error(msg) {}
};

#endif
