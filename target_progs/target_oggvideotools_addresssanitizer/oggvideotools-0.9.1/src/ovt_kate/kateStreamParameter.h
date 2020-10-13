#ifndef KATESTREAMPARAMETER_H_
#define KATESTREAMPARAMETER_H_

#include "definition.h"
#include "streamParameter.h"

class KateStreamParameter : public StreamParameter {
public:

  uint32 granulerateNum;
  uint32 granulerateDenom;
  std::string language;
  std::string category;

  uint8  granuleShift;

  KateStreamParameter();
  virtual ~KateStreamParameter();

  virtual bool operator==(const StreamParameter& param);

  virtual std::string toString();

  virtual StreamParameter* clone();
};

#endif /*KATESTREAMPARAMETER_H_*/
