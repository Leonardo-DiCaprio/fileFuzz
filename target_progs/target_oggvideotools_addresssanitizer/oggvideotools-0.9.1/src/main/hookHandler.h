#ifndef HOOKHANDLER_H_
#define HOOKHANDLER_H_

#include <vector>
#include <deque>

#include "mediaOutputDecoder.h"
#include "mediaInputEncoder.h"
#include "oggComment.h"

class HookHandler {

protected:
  bool copyOnly;           /* specifies, if resize is disallowed */
  bool reencode;
  bool keepComments;
  uint64 inPacketCounter;
  uint64 outPacketCounter;
  std::vector<OggComment> comments;

  std::unique_ptr<MediaOutputDecoder> outputDecoder;
  std::unique_ptr<MediaInputEncoder>  inputEncoder;
  std::deque<OggPacket> packetList;


public:
  HookHandler(const bool copy=true, const bool keepComments=true);
  virtual ~HookHandler();

  virtual void setDecoderConfig(StreamConfig& config,
                                std::vector<OggComment>& commentList);

  virtual void setEncoderConfig(StreamConfig& config,
                                std::vector<OggComment>& commentList);

  virtual void resetEncoder();

  void setCopyOnly();
  bool isCopyOnly();
  void resetCopyOnly();

  void forceReencoding();
  void resetForceReencoding();

  //! method to compare both configurations and to calculate the reencoding parameters
  virtual void initAndConnect() = 0;

  virtual HookHandler& operator<<(OggPacket& packet) = 0;
  virtual HookHandler& operator>>(OggPacket& packet);

  virtual OggType getType() const;
  virtual bool available();

  virtual void flush() = 0;

  virtual std::string decoderConfiguration() const;
  virtual std::string encoderConfiguration() const;
};

inline void HookHandler::setCopyOnly()
{
  copyOnly = true;
}

inline bool HookHandler::isCopyOnly()
{
  return(copyOnly);
}

inline void HookHandler::resetCopyOnly()
{
  copyOnly = false;
}

inline void HookHandler::forceReencoding()
{
  reencode = true;
}

inline void HookHandler::resetForceReencoding()
{
  reencode = false;
}

#endif /*HOOKHANDLER_H_*/
