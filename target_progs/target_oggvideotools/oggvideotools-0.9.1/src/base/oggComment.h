#ifndef OGGCOMMENT_H_
#define OGGCOMMENT_H_

#include <string>

class OggComment {
public:
  OggComment();
  virtual ~OggComment();

  std::string tag;
  std::string value;

};

#endif /*OGGCOMMENT_H_*/
