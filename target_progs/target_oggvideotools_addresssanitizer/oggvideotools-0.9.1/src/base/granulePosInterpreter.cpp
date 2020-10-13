#include "granulePosInterpreter.h"

GranulePosInterpreter::GranulePosInterpreter()
  : initialized(false), actualGranulePosition(0)
{
}

GranulePosInterpreter::~GranulePosInterpreter()
{
}

int64 GranulePosInterpreter::getPosition()
{
  return(actualGranulePosition);
}

double GranulePosInterpreter::getActTime()
{
  return(getTime(actualGranulePosition));
}
