#include <iostream>
#include <sstream>

#include "kateStreamParameter.h"
#include "log.h"

KateStreamParameter::KateStreamParameter()
{
}

KateStreamParameter::~KateStreamParameter()
{
}

bool KateStreamParameter::operator==(const StreamParameter& _param)
{
  StreamParameter* _param_unconst = const_cast<StreamParameter*>(&_param);
  KateStreamParameter* param = dynamic_cast<KateStreamParameter*>(_param_unconst);

  if (!param)
    return(false);

  if (granuleShift != param->granuleShift) {
    logger.error() << "kate parameter compare: the granule shift is not matching\n";
    return(false);
  }

  if ((granulerateNum != param->granulerateNum) ||
      (granulerateDenom != param->granulerateDenom)) {
    logger.error() << "kate parameter compare: granulerate does not match: "
                   << granulerateNum << "/" << param->granulerateDenom
                   << " != " << param->granulerateNum << "/" << param->granulerateDenom
                   << std::endl;
    return(false);
  }

  if (language != param->language) {
    logger.error() << "kate parameter compare: language does not match: "
                   << language << param->language
                   << std::endl;
    return(false);
  }

  if (category != param->category) {
    logger.error() << "kate parameter compare: category does not match: "
                   << category << param->category
                   << std::endl;
    return(false);
  }

  return(true);
}

std::string KateStreamParameter::toString()
{
  std::stringstream stream;

  stream << std::endl;
  stream << "Language     : " << language << "\n";
  stream << "Category     : " << category << "\n";
  stream << "Granulerate  : " << granulerateNum/granulerateDenom << "\n";

  stream << std::endl;

  return(stream.str());

}

StreamParameter* KateStreamParameter::clone()
{
  // create a clone
  KateStreamParameter* streamParameter = new KateStreamParameter();

  // copy the original data to the clone
  (*streamParameter) = (*this);

  // return the clone
  return(streamParameter);

}
