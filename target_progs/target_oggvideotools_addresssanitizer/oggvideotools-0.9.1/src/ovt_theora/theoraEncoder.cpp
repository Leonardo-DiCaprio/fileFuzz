/*
 * TheoraEncoder wrapper
 *
 * Copyright (C) 2008 Joern Seger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "theoraEncoder.h"

#ifdef HAVE_LIBTHEORADEC

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <ogg/ogg.h>

#include "exception.h"
#include "log.h"

TheoraEncoder::TheoraEncoder(uint8 _streamNo)
  : MediaInputEncoder(_streamNo), packetCounter(0)
{
  th_comment_init(&theoraComment);
}

TheoraEncoder::~TheoraEncoder()
{
  if (isConfigured())
    th_encode_free(theoraState);

  th_info_clear(&theoraInfo);
  th_comment_clear(&theoraComment);

}

void TheoraEncoder::createHeader(std::vector<OggPacket>& headerList, std::vector<OggComment>& oggComments)
{
  int32 encodeRetID(1);

  th_comment_add_tag(&theoraComment,"ENCODER",PACKAGE_STRING);

  /* add other comments */
  for (uint32 i(0); i<oggComments.size(); ++i)
    th_comment_add_tag(&theoraComment, (char*) oggComments[i].tag.c_str(), (char*) oggComments[i].value.c_str());

  while (encodeRetID > 0) {
    ogg_packet tmpPkt;
    encodeRetID = th_encode_flushheader(theoraState, &theoraComment, &tmpPkt); //packet->getUnderlayingOggPacketPtr());

    if (encodeRetID == TH_EFAULT)
      throw OggException("TheoraEncoder::operator <<: encoder or packet are NULL");

    //    ost::slog(ost::Slog::levelDebug) << "TheoraEncoder:: inserting header/n";
    if (encodeRetID > 0) {
      OggPacket packet = std::make_shared<OggPacketInternal>(tmpPkt);

#ifdef DEBUG
      logger.debug() << "Theora Packet Number: "<< packet.packetno << "reset to 0" << std::endl;
#endif

      packet->setStreamType(OggType::theora);
      packet->setStreamNo(streamNo);
      packet->setStreamHeader();
      packet->setPacketno(0);

      headerList.push_back(packet);
    }
  }


}

void TheoraEncoder::reset()
{
  if (isConfigured()) {
    th_encode_free(theoraState);
    theoraState = th_encode_alloc(&theoraInfo);
  }

}

void TheoraEncoder::configureEncoder(StreamConfig& streamConf, std::vector<OggComment>& oggComments)
{
  if (isConfigured())
    throw OggException("TheoraEncoder::configureEncoder: can't configure encoder twice");

  TheoraStreamParameter& config = dynamic_cast<TheoraStreamParameter&>(*streamConf.parameter.get());

  // Theora has a divisible-by-sixteen restriction for the encoded video size
  // scale the frame size up to the nearest /16 and calculate offsets

  config.frameX  = (config.pictureX+15)&~0xF;
  config.frameY  = (config.pictureY+15)&~0xF;

  // We force the offset to be even.
  // This ensures that the chroma samples align properly with the luma
  // samples.

//  config.frameXOffset  = ((config.frameX - config.pictureX)/2)&~1;
//  config.frameYOffset  = ((config.frameY - config.pictureY)/2)&~1;
//  config.frameXOffset = 0;
//  config.frameYOffset = 0;

  // let's initialize the theora encoder
  th_info_init(&theoraInfo);

  theoraInfo.pic_width          = config.pictureX;
  theoraInfo.pic_height         = config.pictureY;
  theoraInfo.frame_width        = config.frameX;
  theoraInfo.frame_height       = config.frameY;
  theoraInfo.pic_x              = config.frameXOffset;
  theoraInfo.pic_y              = config.frameYOffset;
  theoraInfo.fps_numerator      = config.framerateNum;
  theoraInfo.fps_denominator    = config.framerateDenom;
  theoraInfo.aspect_numerator   = config.aspectRatioNum;
  theoraInfo.aspect_denominator = config.aspectRatioDenom;
  switch ( config.colorspace ) {
  case TheoraStreamParameter::ITU_470M:
    theoraInfo.colorspace = TH_CS_ITU_REC_470M;
    break;
  case TheoraStreamParameter::ITU_470BG:
    theoraInfo.colorspace = TH_CS_ITU_REC_470BG;
    break;
  default:
    theoraInfo.colorspace = TH_CS_UNSPECIFIED;
    break;
  }
  switch (config.pixel_fmt) {
  case TheoraStreamParameter::pf_420:
    theoraInfo.pixel_fmt      = TH_PF_420;
    break;
  case TheoraStreamParameter::pf_422:
    theoraInfo.pixel_fmt      = TH_PF_422;
    break;
  case TheoraStreamParameter::pf_444:
    theoraInfo.pixel_fmt      = TH_PF_444;
    break;
  default:
    theoraInfo.pixel_fmt      = TH_PF_420; // most likly this format
    break;
  }

  theoraInfo.target_bitrate     = config.videoBitrate;
  theoraInfo.quality            = config.videoQuality;
  theoraInfo.keyframe_granule_shift = config.keyframeShift; // 6 bit to distinguish interframes

  // TODO: Pixel Format should be available in config
  /* create a new theora encoder handle */
  theoraState = th_encode_alloc(&theoraInfo);

  if (theoraState)
    setConfigured();
  else
    throw OggException("TheoraEncoder::setConfig: Parameters invalid");

  createHeader(streamConf.headerList, oggComments);

  streamConf.serialNo = (uint32) rand();
  streamConf.streamNo = streamNo;
  streamConf.type = OggType::theora;
  streamConf.numOfHeaderPackets = (uint8) streamConf.headerList.size();

  // reset the packet counter if encoder is reconfigured
  packetCounter = 0;
}

MediaInputEncoder& TheoraEncoder::operator >>(OggPacket& packet)
{
  if (packetList.empty())
    throw OggException("TheoraEncoder::operator >>: No PacketAvailable");

  packet = packetList.front();
  packetList.pop_front();

  packet->setPacketno(packetCounter++);

  if (packetList.empty())
    setEmpty();

  return(*this);
}

MediaInputEncoder& TheoraEncoder::operator <<(th_ycbcr_buffer buffer)
{
  if (!isConfigured())
    throw OggException("TheoraEncoder::operator <<: codec not configured");

  int32 errID;
  if ((errID = th_encode_ycbcr_in(theoraState, buffer)) != 0) {
    if (errID == TH_EFAULT)
      throw OggException("TheoraEncoder::operator <<: encoder or video buffer is NULL");
    if (errID == TH_EINVAL) {
      logger.debug() << "Size of picture "<<buffer[0].width << " x " << buffer[0].height<< " encoder wants "
                     << std::endl;
      throw OggException("TheoraEncoder::operator <<: buffer size does not match the frame size the encoder was initialized with, or encoding has already completed");
    }
  }

  int32 encodeRetID(1);

  while ( encodeRetID > 0) {

    ogg_packet _packet;
    encodeRetID = th_encode_packetout(theoraState, 0, &_packet);

    /* Spec says:
     * 	_op 	An ogg_packet structure to fill. All of the elements of this structure will be set,
     * 	including a pointer to the video data. The memory for the video data is owned by libtheoraenc,
     * 	and may be invalidated when the next encoder function is called.
     * 	This means, data may be lost and cannot be used in a packet list, so we need to copy,
     * 	this really kills performance ... */
    if (encodeRetID > 0) {
      OggPacket packet = std::make_shared<OggPacketInternal>(_packet);

#ifdef DEBUG
      logger.debug() << "Theora Packet Number: "<< packet.packetno<<std::endl;
      logger.debug() << "Theora Packet Length: "<< packet.bytes<<std::endl;
#endif

      packet->setStreamType(OggType::theora);
      packet->setStreamNo(streamNo);

      packetList.push_back(packet);
    }
  }

  if (encodeRetID == TH_EFAULT)
    throw OggException("TheoraEncoder::operator <<: encoder or packet are NULL");

  setAvailable();

  return(*this);
}

uint32 TheoraEncoder::width() const
{
  return(theoraInfo.pic_width);
}

uint32 TheoraEncoder::height() const
{
  return(theoraInfo.pic_height);
}

th_info& TheoraEncoder::getInfo()
{
  return(theoraInfo);
}

std::string TheoraEncoder::configuration() const
{
  std::stringstream stream;

  stream << "Theora Encoder Configuration:"<<std::endl;
  stream << "Stream No: "<<(int)streamNo<<std::endl;
  stream << std::endl;
  stream << "Theora Version   : " << (int) theoraInfo.version_major << "." << (int) theoraInfo.version_minor
         << "." << (int) theoraInfo.version_subminor << std::endl;
  stream << std::endl;
  stream << "Video Size       : " << theoraInfo.pic_width << " x " << theoraInfo.pic_height<<std::endl;

  if ((theoraInfo.pic_width != theoraInfo.frame_width) ||
      (theoraInfo.pic_height != theoraInfo.frame_height)) {

    stream << " - Frame Size    : " << theoraInfo.frame_width << " x " << theoraInfo.frame_height << std::endl;
    stream << " - Offset        : " << theoraInfo.pic_x << " x " << theoraInfo.pic_y << std::endl;

  }

  stream << "Keyframe Shift   : " <<(uint32)(1 << theoraInfo.keyframe_granule_shift)<< " frames " << std::endl;
  stream << "Aspect Ratio     : " << theoraInfo.aspect_numerator << " : " << theoraInfo.aspect_denominator << std::endl;
  stream << "Framerate        : " << theoraInfo.fps_numerator << " / " << theoraInfo.fps_denominator << "\n";
  stream << std::endl;
  stream << "Quality          : " << theoraInfo.quality << " / 64" << std::endl;
  stream << "Datarate         : " << theoraInfo.target_bitrate << std::endl;
  stream << "Pixel Format     : ";
  switch (theoraInfo.pixel_fmt) {
  case TH_PF_420:
    stream << "420 (Chroma decimination by 2 in both directions)"<<std::endl;
    break;
  case TH_PF_422:
    stream << "422 (Chroma decimination by 2 in X direction)"<<std::endl;
    break;
  case TH_PF_444:
    stream << "444 (No Chroma decimination)"<<std::endl;
    break;
  default:
    stream << " unknown"<<std::endl;
    break;
  }
  stream << "Colorspace       : ";
  switch (theoraInfo.colorspace) {
  case TH_CS_ITU_REC_470M:
    stream << "ITU Rec. 470M (designed for NTSC content)"<<std::endl;
    break;
  case TH_CS_ITU_REC_470BG:
    stream << "ITU Rec. 470BG (designed for PAL/SECAM content)"<<std::endl;
    break;
  default:
    stream << "unspecified"<<std::endl;
    break;
  }

  stream << std::endl;

  if (theoraComment.comments) {
    stream << "Comments:\n";
    for (int i=0; i<theoraComment.comments; ++i)
      stream << theoraComment.user_comments[i] << std::endl;

    stream << std::endl;
  }

  return(stream.str());
}

#endif


