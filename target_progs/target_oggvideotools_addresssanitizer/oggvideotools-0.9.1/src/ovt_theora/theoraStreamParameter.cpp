#include <iostream>
#include <sstream>

#include "theoraStreamParameter.h"
#include "log.h"

TheoraStreamParameter::TheoraStreamParameter()
{
}

TheoraStreamParameter::~TheoraStreamParameter()
{
}

bool TheoraStreamParameter::operator==(const StreamParameter& _param)
{
  StreamParameter* _param_unconst = const_cast<StreamParameter*>(&_param);
  TheoraStreamParameter* param = dynamic_cast<TheoraStreamParameter*>(_param_unconst);

  if (!param)
    return(false);

  if (keyframeShift != param->keyframeShift) {
    logger.error() << "theora parameter compare: the granule shift is not matching\n";
    return(false);
  }

  if ((framerateNum != param->framerateNum) ||
      (framerateDenom != param->framerateDenom)) {
    logger.error() << "theora parameter compare: framerate does not match: "
                   << framerateNum << "/" << param->framerateDenom
                   << " != " << param->framerateNum << "/" << param->framerateDenom
                   << std::endl;
    return(false);
  }

  if ((pictureX != param->pictureX) ||
      (pictureY  != param->pictureY)) {
    logger.error() << "theora parameter compare: height or width are not matching:"
                   << pictureX << ":" << pictureY << " != "
                   << param->pictureX << ":" << param->pictureY << "\n";
    return(false);
  }

  if ((frameX != param->frameX) ||
      (frameY != param->frameY)) {
    logger.error() << "theora parameter compare: frame height or width are not matching:"
                   << frameX << ":" << frameY << " != "
                   << param->frameX << ":" << param->frameY << "\n";
    return(false);
  }

  /*  if ((frameXOffset != param->frameXOffset) ||
        (frameYOffset != param->frameYOffset)) {
      logger.error() << "theora parameter compare: offsets are not matching:"
                     << frameXOffset << ":" << frameXOffset << " != "
                     << param->frameYOffset << ":" << param->frameYOffset << "\n";
      return(false);
    }
  */
  return(true);
}

std::string TheoraStreamParameter::toString()
{
  std::stringstream stream;

  stream << std::endl;
  stream << "Size         : " << pictureX << " x " << pictureY
         << " (Frame Size : " << frameX << " x" << frameY << " )\n"
         << "KeyframeShift: " <<(uint32)keyframeShift<<std::endl;
  stream << "Aspect Ratio : " << aspectRatioNum << ":" << aspectRatioDenom << std::endl;
  stream << "Framerate    : " << framerateNum <<"/"<<framerateDenom << "\n";

  stream << "Offset       : " << frameXOffset << ":"<<frameYOffset<<std::endl;

  stream << "Quality      : " << videoQuality << " / 61" << std::endl;
  stream << "Datarate     : " << videoBitrate << std::endl;

  stream << std::endl;

  return(stream.str());

}

StreamParameter* TheoraStreamParameter::clone()
{
  // create a clone
  TheoraStreamParameter* streamParameter = new TheoraStreamParameter();

  // copy the original data to the clone
  (*streamParameter) = (*this);

  // return the clone
  return(streamParameter);

}

void TheoraStreamParameter::calculateFrame()
{
  frameX  = (pictureX+15)&~0xF;
  frameY  = (pictureY+15)&~0xF;
  frameXOffset     = ((frameX - pictureX)/2)&~1;
  frameYOffset     = ((frameY - pictureY)/2)&~1;
}
