/*
 * TheoraDecoder wrapper
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

#include "theoraDecoder.h"
#include <climits>

#ifdef HAVE_LIBTHEORADEC

#include "theoraStreamParameter.h"
#include "exception.h"

#include <iostream>
#include <sstream>
#include <string>

#include <theora/codec.h>
#include <theora/theoradec.h>
#include <ogg/ogg.h>
#include "exception.h"
#include "log.h"

TheoraDecoder::TheoraDecoder(uint8 _streamID) :
  MediaOutputDecoder(_streamID), setupInfo(NULL), theoraDecState(NULL),
  initCount(0)
{
}

TheoraDecoder::~TheoraDecoder()
{
  clear();
}

void TheoraDecoder::clear()
{
  if (isConfigured()) {
    /* delete all packets, that may be left within the queue */
    packetList.clear();

    /* clean up everything regarding the decoding process */
    th_setup_free(setupInfo);
    setupInfo=0;
    th_decode_free(theoraDecState);
    theoraDecState=0;
    th_info_clear(&theoraInfo);
    th_comment_clear(&theoraComment);
    setFree();
  }
}

void TheoraDecoder::initDecoder(StreamConfig& config,
                                std::vector<OggComment>& oggComments)
{
  if (isConfigured())
    throw OggException("TheoraDecoder::initDecoder: could not configure twice");


  /* initialize the info and comment handler structs */
  th_info_init(&theoraInfo);
  th_comment_init(&theoraComment);

  /* initialize the packet counter */
  packetCount = 0;

  /* configure the decoders */
  for (uint8 i(0); i<config.headerList.size(); ++i) {

    /* Insert the header
     * Give an error message if the data does not fit the current codec */
    int retVal = th_decode_headerin(&theoraInfo, &theoraComment,
                                    &setupInfo, config.headerList[i]->getUnderlayingOggPacketPtr());
    if (retVal <= 0) {
      // TODO: cleanup everything that has been set up until now
      th_comment_clear(&theoraComment);
      if (retVal == 0) {
        // There was a video packet in config.headerList. This should not happen,
        // because we only read 3 packets (numOfHeaderPackets)
        throw OggException("TheoraDecoder::initDecoder: unexpected video packet");
      } else {
        reportTheoraError(retVal);
      }
    }

  }

  /* extract the comments */
  for (uint8 i(0); i<theoraComment.comments; ++i) {
    /* We have to extract the tags by ourself */
    std::string commentStr(theoraComment.user_comments[i],
                           theoraComment.comment_lengths[i]);

    std::size_t commentSeparatorPos;
    if ((commentSeparatorPos = commentStr.find_first_of("="))
        != std::string::npos) {
      OggComment comment;
      comment.tag = commentStr.substr(0, commentSeparatorPos);
      comment.value = commentStr.substr(commentSeparatorPos+1,
                                        std::string::npos);
      oggComments.push_back(comment);
    }

  }

  th_comment_clear(&theoraComment);

  /* finish initialization, if the frame does exceed 4096x4096 Pixel
   it will not be handled. This must handle integer overflow. */
  if ( (theoraInfo.frame_height>INT_MAX/theoraInfo.frame_width)
       || ((theoraInfo.frame_width*theoraInfo.frame_height)
           >= maxVideoPlaneSize)) {
    throw OggException("TheoraDecoder::initDecoder: could not set up decoder, frame is too big");
  }

  theoraDecState = th_decode_alloc(&theoraInfo, setupInfo);
  if (theoraDecState == NULL) {
    throw OggException("TheoraDecoder::initDecoder: th_decode_alloc failed");
  }

  TheoraStreamParameter& theoraConfig = dynamic_cast<TheoraStreamParameter&>(*config.parameter.get());

  theoraConfig.aspectRatioDenom = theoraInfo.aspect_denominator;
  theoraConfig.aspectRatioNum = theoraInfo.aspect_numerator;
  theoraConfig.colorspace = (TheoraStreamParameter::ColorSpace) theoraInfo.colorspace;
  theoraConfig.framerateDenom = theoraInfo.fps_denominator;
  theoraConfig.framerateNum   = theoraInfo.fps_numerator;
  theoraConfig.pictureX    = theoraInfo.pic_width;
  theoraConfig.pictureY    = theoraInfo.pic_height;
  theoraConfig.frameX      = theoraInfo.frame_width;
  theoraConfig.frameY      = theoraInfo.frame_height;
  theoraConfig.frameXOffset = theoraInfo.pic_x;
  theoraConfig.frameYOffset = theoraInfo.pic_y;
  theoraConfig.keyframeShift = theoraInfo.keyframe_granule_shift;
  theoraConfig.pixel_fmt    = (TheoraStreamParameter::PixFormat) theoraInfo.pixel_fmt;
  theoraConfig.videoBitrate = theoraInfo.target_bitrate;
  theoraConfig.videoQuality = theoraInfo.quality;

  /* set the state machine */
  setConfigured();

}

MediaOutputDecoder& TheoraDecoder::operator<<(OggPacket packet)
{
  /* if the stream is not initialized, initialize the first structs */
  if (!isConfigured())
    throw OggException("TheoraDecoder::initDecoder: Theora decoder is not configured");

  /* while inserting data into the stream, we do not
   * decode. We just store the packets and will decode them
   * on demand */
  packetList.push_back(packet);

  /* has there not been a packet in the queue before */
  if (isEmpty()) {

    /* set the internal state */
    setAvailable();

  }

  /* count the video packets, to have a gimps of the actual position */
  packetCount++;
  return(*this);
}

bool TheoraDecoder::isNextPacketKeyframe()
{
  return ((th_packet_iskeyframe(packetList.front()->getUnderlayingOggPacketPtr()) == 1));
}

bool TheoraDecoder::isPacketKeyframe(OggPacket packet)
{
  return ((th_packet_iskeyframe(packet->getUnderlayingOggPacketPtr()) == 1));
}

uint32 TheoraDecoder::getPositionOfNextPacket()
{
  if (isEmpty())
    return (0xFFFFFFFF);

  return (packetCount - packetList.size());
}

TheoraDecoder& TheoraDecoder::operator>>(th_ycbcr_buffer& picture)
{
  if (!isConfigured())
    throw OggException("TheoraDecoder::operator>>: Theora decoder is not configured");

  if (isEmpty())
    throw OggException("TheoraDecoder::operator>>: No packet available");

  /* get the first packet from the packet list */
  OggPacket packet = packetList.front();
  packetList.pop_front();

  /* insert the packet into the theora decoder */
  ogg_int64_t dummy;
  int result;
  result = th_decode_packetin(theoraDecState, packet->getUnderlayingOggPacketPtr(), &dummy);
  if (result != 0 && result != TH_DUPFRAME) {
    reportTheoraError(result);
  }

  /* finally decode the picture */
  result = th_decode_ycbcr_out(theoraDecState, picture);
  if (result != 0) {
    reportTheoraError(result);
  }

  if (packetList.empty()) {
    setEmpty();
  }

  return(*this);
}

/*std::string TheoraDecoder::getInfoString()
{

	std::stringstream stream;

	if (!isConfigured())
	{
		logger.error()
				<< "TheoraDecoder::operator>>: Theora Stream is not initialized\n";
		return (stream.str());
	}

	stream << std::endl;
	stream << "Size         : " << theoraInfo.pic_width << ":"
			<< theoraInfo.pic_height << " (Frame Size : "
			<< theoraInfo.frame_width << ":" << theoraInfo.frame_height
			<< " ; Offset: "<<theoraInfo.pic_x<<":"<<theoraInfo.pic_y<<" \n";
	stream << "Aspect Ratio : " << theoraInfo.aspect_numerator << ":"
			<< theoraInfo.aspect_denominator << std::endl;
	stream << "Framerate    : " << theoraInfo.fps_numerator
			/theoraInfo.fps_denominator << "\n";

	stream << "Colorspace   : ";
	switch (theoraInfo.colorspace)
	{
	case TH_CS_ITU_REC_470M:
		stream << "NTSC\n";
		break;
	case TH_CS_ITU_REC_470BG:
		stream << "PAL\n";
		break;
	default:
		stream << "unspecified\n";
	}

	stream << "Quality      : " << theoraInfo.quality << " / 61" << std::endl;
	stream << "Data rate    : " << theoraInfo.target_bitrate << " kBit/s"
			<< std::endl;

	stream << std::endl;

	stream << "Comments:\n";
	for (int i=0; i<theoraComment.comments; ++i)
		stream << theoraComment.user_comments[i] << std::endl;

	return (stream.str());
}
*/

th_info& TheoraDecoder::getInfo()
{
  return (theoraInfo);
}

th_comment& TheoraDecoder::getComment()
{
  return (theoraComment);
}

uint32 TheoraDecoder::getWidth()
{
  return (theoraInfo.pic_width);
}

uint32 TheoraDecoder::getHeight()
{
  return (theoraInfo.pic_height);
}

std::string TheoraDecoder::configuration() const
{
  std::stringstream stream;

  stream << "Theora Decoder Configuration:"<<std::endl;
  stream << std::endl;
  stream << "Theora Version   : " << (int)theoraInfo.version_major << "."
         << (int)theoraInfo.version_minor << "." << (int)theoraInfo.version_subminor
         << std::endl;
  stream << std::endl;
  stream << "Video Size       : " << theoraInfo.pic_width << " x "
         << theoraInfo.pic_height<<std::endl;

  if ((theoraInfo.pic_width != theoraInfo.frame_width)
      || (theoraInfo.pic_height != theoraInfo.frame_height)) {

    stream << " - Frame Size    : " << theoraInfo.frame_width << " x "
           << theoraInfo.frame_height << std::endl;
    stream << " - Offset        : " << theoraInfo.pic_x << " x "
           << theoraInfo.pic_y << std::endl;

  }

  stream << "Keyframe Shift   : " <<(uint32)(1
         << theoraInfo.keyframe_granule_shift)<<" frames "<<std::endl;
  stream << "Aspect Ratio     : " << theoraInfo.aspect_numerator << " : "
         << theoraInfo.aspect_denominator << std::endl;
  stream << "Framerate        : " << theoraInfo.fps_numerator << " / "
         << theoraInfo.fps_denominator << "\n";
  stream << std::endl;
  stream << "Quality          : " << theoraInfo.quality << " / 64"
         << std::endl;
  stream << "Datarate         : " << theoraInfo.target_bitrate << std::endl;
  stream << "Pixel Format     : ";
  switch (theoraInfo.pixel_fmt) {
  case TH_PF_420:
    stream << "420 (Chroma decimination by 2 in both directions)"
           <<std::endl;
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
    stream << "unspecified\n";
    break;
  }

  stream << std::endl;

  if (theoraComment.comments) {
    stream << "Comments:\n";
    for (int i=0; i<theoraComment.comments; ++i)
      stream << theoraComment.user_comments[i] << std::endl;

    stream << std::endl;
  }

  return (stream.str());
}

void TheoraDecoder::reportTheoraError(int error) const
{
  std::string msg;
  switch (error) {
  case TH_EBADHEADER:
    msg = "bad header";
    break;
  case TH_EVERSION:
    msg = "the bitstream version is not supported by this version of libtheoradec";
    break;
  case TH_ENOTFORMAT:
    msg = "the packet was not a Theora header";
    break;
  case TH_EBADPACKET:
    msg = "the packet does not contain encoded video data";
    break;
  case TH_EIMPL:
    msg = "the video uses bitstream features which this library does not support";
    break;
  default:
    msg = "unknown error";
  }
  throw OggException(std::string("Error decoding Theora data: ") + msg);
}

#endif
