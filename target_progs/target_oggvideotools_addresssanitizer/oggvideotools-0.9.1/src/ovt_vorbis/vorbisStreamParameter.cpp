#include <iostream>
#include <sstream>

#include "vorbisStreamParameter.h"
#include "log.h"

VorbisStreamParameter::VorbisStreamParameter()
{
}

VorbisStreamParameter::~VorbisStreamParameter()
{
}

bool VorbisStreamParameter::operator==(const StreamParameter& _param)
{
  StreamParameter* _param_unconst = const_cast<StreamParameter*>(&_param);
  VorbisStreamParameter* param = dynamic_cast<VorbisStreamParameter*>(_param_unconst);

  bool retValue(true);

  if (!param)
    return(false);

  if (channels != param->channels) {
    logger.error() << "vorbis parameter compare: number of channels not matching "
                   << channels << " != " << param->channels
                   << std::endl;
    retValue = false;
  }

  if (samplerate != param->samplerate) {
    logger.error() << "vorbis parameter compare: sample rate not matching "
                   << samplerate << " != " << param->samplerate
                   << std::endl;
    retValue = false;
  }

  if (datarate != param->datarate) {
    logger.error() << "vorbis parameter compare: data rate not matching "
                   << datarate << " != " << param->datarate << std::endl;
//		<< "This is not a blocker if all other parameters match" <<std::endl;
    retValue = false;
  }

  if (block0 != param->block0) {
    logger.error() << "vorbis parameter compare: size of block0 does not match "
                   << block0 << " != " << param->block0 << std::endl
                   << "You may try to reencode with the datarate of the other file"
                   << std::endl;
    retValue = false;
  }

  if (block1 != param->block1) {
    logger.error() << "vorbis parameter compare: size of block0 does not match "
                   << block1 << " != " << param->block1 << std::endl
                   << "You may try to reencode with the datarate of the other file"
                   << std::endl;
    retValue = false;
  }

  if (retValue == false) {
    logger.error() << "\nPlease try to resample with the following command\n"
                   << "oggResize";
    if (channels != param->channels)
      logger.error() << " -N "<<channels;
    if (samplerate != param->samplerate)
      logger.error() << " -F "<<samplerate;
    if (datarate != param->datarate)
      logger.error() << " -D "<<datarate;
    logger.error() <<" <file see below>\n\n";
  }

  return(retValue);
}

std::string VorbisStreamParameter::toString()
{
  std::stringstream stream;
  stream << "Vorbis Stream:\n" << "\twith " << channels << " channel(s)\n"
         << "\tand  " << samplerate << " kHz sample rate\n" << "\tand  "
         << datarate << " data rate\n\n";

//  logger.debug() << "block0: "<<block0<<"\nblock1: "<<block1<<"\n\n";
  return (stream.str());
}

StreamParameter* VorbisStreamParameter::clone()
{
  // create a clone object
  VorbisStreamParameter* streamParameter = new VorbisStreamParameter();

  // copy the original data to the clone
  (*streamParameter) = (*this);

  // return the clone
  return (streamParameter);
}
