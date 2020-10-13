#include "exception.h"
#include "hookHandler.h"

HookHandler::HookHandler(const bool _copy, const bool _keepComments)
  : copyOnly(_copy), reencode(false), keepComments(_keepComments), inPacketCounter(0), outPacketCounter(0)
{
}

HookHandler::~HookHandler()
{
}

/* refactor StreamConfig to be const*/
void HookHandler::setDecoderConfig(StreamConfig& config,
                                   std::vector<OggComment>& commentList)
{
  if (!outputDecoder)
    throw OggException("No decoder available to configure");

  outputDecoder->clear();
  outputDecoder->initDecoder(config, commentList);

}

void HookHandler::setEncoderConfig(StreamConfig& config,
                                   std::vector<OggComment>& commentList)
{
  if (!inputEncoder)
    throw OggException("No encoder available to configure");

  inputEncoder->configureEncoder(config, commentList);
}

std::string HookHandler::decoderConfiguration() const
{
  return (outputDecoder->configuration());
}

std::string HookHandler::encoderConfiguration() const
{
  return (inputEncoder->configuration());
}

HookHandler& HookHandler::operator>>(OggPacket& packet)
{
  if (packetList.empty())
    throw OggException("VideoHook::operator>>: No packet available");

  packet = packetList.front();
  packetList.pop_front();

  return (*this);
}

bool HookHandler::available()
{
  return (!packetList.empty());
}

OggType HookHandler::getType() const
{
  return (OggType::unknown);
}

void HookHandler::resetEncoder()
{

}
