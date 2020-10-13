#ifndef WISHLIST_H
#define WISHLIST_H

struct WishList {

  uint32 width;
  uint32 height;
  uint32 offsetX;
  uint32 offsetY;

  bool changeSize;

  uint32 framerateNum;
  uint32 framerateDenom;
  bool changeFramerate;

  uint32 videoDatarate;
  bool changeVideoDatarate;

  uint32 videoQuality;
  bool changeVideoQuality;

  uint32 audioDatarate;
  bool changeAudioDatarate;

  uint32 audioSamplerate;
  bool changeAudioSamplerate;

  uint32 audioChannels;
  bool changeAudioChannels;

  bool stretch;
  bool withBlend;
  bool ignoreVorbis;

  uint32 preview;
  uint32 pictureCounter;
  uint8 quality;

  WishList() :
    width(0), height(0), changeSize(false), framerateNum(1),
    framerateDenom(1), changeFramerate(false), videoDatarate(0),
    changeVideoDatarate(false), videoQuality(0),
    changeVideoQuality(false), audioDatarate(0),
    changeAudioDatarate(false), audioSamplerate(0),
    changeAudioSamplerate(false), audioChannels(2),
    changeAudioChannels(false), stretch(false), withBlend(false),
    ignoreVorbis(false), preview(1), pictureCounter(0), quality(2) {
  }

};

#endif
