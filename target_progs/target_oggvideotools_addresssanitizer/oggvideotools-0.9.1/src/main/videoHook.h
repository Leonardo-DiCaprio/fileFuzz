#ifndef VIDEOHOOK_H_
#define VIDEOHOOK_H_

// #include <deque>

#include "hookHandler.h"

#include "theoraDecoder.h"
#include "theoraEncoder.h"
#include "rgbPlane.h"
#include "theoraPosInterpreter.h"

#include "blendElement.h"

class VideoHook : public HookHandler {
public:

  class Config {
  public:
    bool stretch;
    bool trimDatarateOrQuality;
    uint32 quality;
    uint32 preview;
    std::vector<BlendElement> blendListBefore;
    std::vector<BlendElement> blendListAfter;
    Config();
  };

private:
  Config config;

  /* precalculations from known values */
  double framerateDecoder;
  double framerateEncoder;

  double aspectCorrection;
  double time;
  double nextTime;
  double timeOffset;

  double intensityStair;

  bool copy;

  bool changeSize;
  RGBPlane inPlane;

  TheoraPosInterpreter posCreator;

  th_ycbcr_buffer inycbcr;
  th_ycbcr_buffer outycbcr;

  VideoHook();

  void alphaBlend(double time, RGBPlane& inPlane,
                  std::vector<BlendElement>& blendList);

public:

  VideoHook(uint8 outStreamID, const bool copy=true, const bool keepComments=true);
  virtual ~VideoHook();

  void configureProcess(Config& config);

  virtual void setEncoderConfig(StreamConfig& config,
                                std::vector<OggComment>& commentList);

  virtual void resetEncoder();

  virtual HookHandler& operator<<(OggPacket& packet);

  virtual void initAndConnect();

  virtual OggType getType() const;

  virtual void flush();

};

static bool operator==(const th_info& info1, const th_info& info2);
static bool operator!=(const th_info& info1, const th_info& info2);

#endif /*VIDEOHOOK_H_*/
