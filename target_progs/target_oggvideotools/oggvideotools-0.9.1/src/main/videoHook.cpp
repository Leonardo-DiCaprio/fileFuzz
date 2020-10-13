#include <iostream>

#include <stdlib.h>

#include "videoHook.h"

#include "theoraDecoder.h"
#include "theoraEncoder.h"

#include "rgbPlane.h"
#include "pictureLoader.h"
#include "pictureResize.h"
#include "pictureBlend.h"
#include "th_helper.h"
#include "exception.h"
#include "log.h"

VideoHook::Config::Config()
  : stretch(false), trimDatarateOrQuality(false), quality(3), preview(1)
{
}

VideoHook::VideoHook(uint8 outStreamID, const bool copy, const bool keepComments) :
  HookHandler(copy, keepComments), framerateDecoder(1), framerateEncoder(1),
  aspectCorrection(1), time(0), nextTime(0), timeOffset(0), intensityStair(1),
  changeSize(false)
{
  config.stretch = false;   // Try to keep the aspect ratio
  config.quality = 3;       // use good quality for resizing
  config.preview = 1;       // don't use any preview functionality

  // we create the decoder/encoder pair,
  // even if we don't need them
  // specially the encoder may be needed later, if the
  // input comes from another source

  outputDecoder.reset(new TheoraDecoder);
  inputEncoder.reset(new TheoraEncoder(outStreamID));

  th_clean_ycbcr(outycbcr);
  th_clean_ycbcr(inycbcr);

}

VideoHook::~VideoHook()
{
  th_free_ycbcr(outycbcr);
  //  th_free_ycbcr(inycbcr);
}

HookHandler& VideoHook::operator<<(OggPacket& packet)
{

  if (!outputDecoder)
    throw OggException("VideoHook::callHook: no outputDecoder given");

  if (!inputEncoder)
    throw OggException("VideoHook::callHook: no inputEncoder given");

  TheoraDecoder& decoder = static_cast<TheoraDecoder&>(*outputDecoder.get());
  TheoraEncoder& encoder = static_cast<TheoraEncoder&>(*inputEncoder.get());

  // What is the best way to receive the time?!
  // best way may be to recalculate the time due to the number of packets
  // and the framerate
  time = (inPacketCounter*framerateEncoder) + timeOffset;
  nextTime = (outPacketCounter*framerateDecoder) + timeOffset;
//  logger.debug() << "Time "<<time<<" Next Time "<<nextTime<<" counterIn "<<inPacketCounter<<" counterOut "<<outPacketCounter<<std::endl;
  inPacketCounter++;

  if (copy) {
    packet->setStreamNo(encoder.getStreamNo());
    // in case, this is just a packet transfer, we need to create
    // the correct position
    if (TheoraPosInterpreter::packetIsKeyframe(packet))
      posCreator.addKeyframe();
    else
      ++posCreator;

    packet->setGranulepos(posCreator.getPosition());
    outPacketCounter++;
    packetList.push_back(packet);

  } else {
    try {
      decoder << packet;
      decoder >> inycbcr;

      while ( (uint64)(time*1000.0+0.5) >= (uint64)(nextTime*1000.0+0.5) ) {

        inPlane = PictureLoader::importYCrCb_theora ( inycbcr, decoder.getWidth(), decoder.getHeight(), decoder.getInfo().pic_x, decoder.getInfo().pic_y, decoder.getInfo().pixel_fmt );

        /* should be an alpha blend applied before resizing */
        if ( !config.blendListBefore.empty() ) {
          alphaBlend ( time, inPlane, config.blendListBefore );
        }

        if ( changeSize ) {
          if (config.stretch)
            inPlane = PictureResize::resize ( inPlane, encoder.width(), encoder.height(), config.quality );
          else
            inPlane = PictureResize::reframe ( inPlane, encoder.width(), encoder.height(), config.quality, 0, aspectCorrection );
        }

        /* should be an alpha blend applied after resizing? */
        if ( !config.blendListAfter.empty() ) {
          alphaBlend ( time, inPlane, config.blendListAfter );
        }

        if ( ( !config.blendListBefore.empty() ) || ( !config.blendListAfter.empty() ) || changeSize ) {

          /* there are changes written to the outycbcr */
          PictureLoader::exportYCrCb_theora ( inPlane, outycbcr, encoder.getInfo().pixel_fmt );

          if (inPacketCounter%config.preview == 0)
            encoder << outycbcr;
        } else {

          /* use the original data */
          if (outPacketCounter%config.preview == 0)
            encoder << inycbcr;
        }

        if (encoder.isAvailable()) {
          outPacketCounter++;
          encoder >> packet;

          if (TheoraPosInterpreter::packetIsKeyframe(packet))
            posCreator.addKeyframe();
          else
            ++posCreator;

          packet->setGranulepos(posCreator.getPosition());
          packetList.push_back(packet);
          nextTime = (outPacketCounter*framerateDecoder) + timeOffset;

        }

      }
      // logger.debug() << std::endl;
    } catch (std::exception & e) {
      logger.error() << "Exception: " << e.what();
    }
  }

  return(*this);
}

void VideoHook::setEncoderConfig(StreamConfig& config,
                                 std::vector<OggComment>& commentList)
{
  HookHandler::setEncoderConfig(config, commentList);
  posCreator.initialize(config.parameter.get());
}

void VideoHook::resetEncoder()
{
  TheoraEncoder& encoder = static_cast<TheoraEncoder&>(*inputEncoder);
  encoder.reset();
}

void VideoHook::initAndConnect()
{
  // maybe we could copy the stream even, when it's not strictly
  // specified
  // at this point every thing must be configured.
  // So the comparison could take place
  TheoraDecoder& decoder = static_cast<TheoraDecoder&>(*outputDecoder.get());
  TheoraEncoder& encoder = static_cast<TheoraEncoder&>(*inputEncoder.get());

  /* if this is a reinitialisation, remember the offset */
  timeOffset = time;
  inPacketCounter = 0;
  outPacketCounter = 0;

  if (!config.blendListAfter.empty() || !config.blendListBefore.empty())
    reencode = true;

  /* if it is ok, that we do a reencoding, than we could trim the output */
  if (!copyOnly) {
    /* if both stream configurations are equal, they could be copied */
    copy = (decoder.getInfo() == encoder.getInfo()) && !reencode;

    /* maybe only the datarate/quality is different in this case, we can copy the
     * stream. But maybe somebody wants to trim the datarate/quality. In this case
     * we do not copy (very difficult ;-)*/

    /* if the picture size is different, we need to resize the video */
    if ((decoder.getInfo().pic_width != encoder.getInfo().pic_width) ||
        (decoder.getInfo().pic_height != encoder.getInfo().pic_height))
      changeSize = true;

    /* if the aspect ratio is different, we also need resizing */
    if ((decoder.getInfo().aspect_numerator != encoder.getInfo().aspect_numerator) ||
        (decoder.getInfo().aspect_denominator != encoder.getInfo().aspect_denominator))
      changeSize = true;

    /* calculate the framerate Input and framerate Output */
    if (decoder.getInfo().fps_denominator > 0)
      framerateDecoder = decoder.getInfo().fps_numerator*1.0/decoder.getInfo().fps_denominator;
    else
      framerateDecoder = 1;

    if (encoder.getInfo().fps_denominator > 0)
      framerateEncoder = encoder.getInfo().fps_numerator*1.0/encoder.getInfo().fps_denominator;
    else
      framerateEncoder = 1;

    // logger.debug() << "changeing framerate from "<< framerateInput<<" to "<<framerateOutput<<std::endl;

    /* We do not use any aspect ratio other than 1 for the output, so
     * we need a correction factor */
    if ((decoder.getInfo().aspect_numerator <= 0) ||
        (decoder.getInfo().aspect_denominator <= 0) )
      aspectCorrection = 1;
    else
      aspectCorrection = (decoder.getInfo().aspect_numerator*1.0)/(decoder.getInfo().aspect_denominator*1.0);

  } else {
    copy = true;
  }

}


void VideoHook::configureProcess(VideoHook::Config& _config)
{
  config = _config;
}

/* you can create a alpha blend object with the following option
 * -a picturex.png,1.23,2.34;picturey.png,5.12,7,s */

void VideoHook::alphaBlend(double time, RGBPlane& inPlane,
                           std::vector<BlendElement>& blendList)
{

  for (uint32 i( 0); i<blendList.size(); ++i) {
    switch (blendList[i].state) {
    case BlendElement::blend_off: {
      if (time >= blendList[i].startTime) {
        if (blendList[i].smooth) {
          blendList[i].state = BlendElement::blend_slideIn;
        } else {
          blendList[i].intensity = 1.0;
          blendList[i].state = BlendElement::blend_on;
        }
      }
    }
    break;

    case BlendElement::blend_slideIn: {
      blendList[i].intensity += intensityStair;

      if (blendList[i].intensity >= 1.0) {
        blendList[i].state = BlendElement::blend_on;
        blendList[i].intensity = 1.0;
      }

    }
    break;

    case BlendElement::blend_on: {
      if ( (blendList[i].endTime > 0.0 )
           && (time >= blendList[i].endTime )) {
        if (blendList[i].smooth) {
          blendList[i].state = BlendElement::blend_slideOut;
        } else {
          blendList[i].intensity = 0.0;
          blendList[i].state = BlendElement::blend_end;
        }
      }
    }
    break;

    case BlendElement::blend_slideOut: {
      blendList[i].intensity -= intensityStair;

      if (blendList[i].intensity <= 0.0) {
        blendList[i].state = BlendElement::blend_end;
        blendList[i].intensity = 0.0;
      }

    }
    break;

    case BlendElement::blend_end: {
      /* do nothing */
    }
    break;

    }

    if ( (blendList[i].state != BlendElement::blend_end )
         && (blendList[i].state != BlendElement::blend_off ))
      inPlane = PictureBlend::alphaBlend(inPlane, blendList[i].picture,
                                         blendList[i].intensity);

  }

}

void VideoHook::flush()
{

}

OggType VideoHook::getType() const
{
  return(OggType::theora);
}

static bool operator==(const th_info& info1, const th_info& info2)
{
  return ( (info1.aspect_denominator == info2.aspect_denominator) &&
           (info1.aspect_numerator == info2.aspect_numerator) &&
           (info1.colorspace == info2.colorspace) &&
           (info1.fps_denominator == info2.fps_denominator ) &&
           (info1.fps_numerator == info2.fps_numerator) &&
           (info1.frame_height == info2.frame_height) &&
           (info1.frame_width == info2.frame_width) &&
           (info1.keyframe_granule_shift == info2.keyframe_granule_shift) &&
           (info1.pic_height == info2.pic_height) &&
           (info1.pic_width == info2.pic_width) &&
           (info1.pic_x == info2.pic_x) &&
           (info1.pic_y == info2.pic_y) &&
           (info1.pixel_fmt == info2.pixel_fmt) &&
           (info1.quality == info2.quality) &&
           (info1.target_bitrate == info2.target_bitrate) );
}

static bool operator!=(const th_info& info1, const th_info& info2)
{
  return (!(info1==info2));
}
