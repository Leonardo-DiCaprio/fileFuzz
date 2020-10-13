#include "vorbisDecoder.h"

#ifdef HAVE_LIBVORBIS

#include <iostream>
#include <sstream>
#include <string>

#include "vorbisStreamParameter.h"
#include "exception.h"
#include "log.h"

VorbisDecoder::VorbisDecoder(uint8 _streamID) :
  MediaOutputDecoder(_streamID), initCount(0)
{

}

VorbisDecoder::~VorbisDecoder()
{
  clear();
}

void VorbisDecoder::clear()
{

  if (isConfigured()) {
    /* delete all packets that may have left within the decoder queue */
    packetList.clear();

    vorbis_info_clear(&vorbisInfo);
    vorbis_block_clear(&vorbisBlock);
    vorbis_dsp_clear(&vorbisDspState);
    vorbis_comment_init(&vorbisComment);
    setFree();
  }

}

void VorbisDecoder::initDecoder(StreamConfig& config, std::vector<OggComment>& oggComments)
{

  if (isConfigured()) {
    throw OggException("VorbisDecoder: Decoder is still configured");
  }

  /* initialize the info and comment handler structs */
  vorbis_info_init(&vorbisInfo);
  vorbis_comment_init(&vorbisComment);

  /* initialize the packet counter */
  packetCount = 0;

  /* Konfiguration des Decoders */
  int retVal;
  for (uint8 i(0); i<config.headerList.size(); ++i) {

    /* Einfügen der Header
     * Fehlermeldung, wenn die Daten nicht zum aktuellen Codec passen */
    retVal = vorbis_synthesis_headerin(&vorbisInfo, &vorbisComment, config.headerList[i]->getUnderlayingOggPacketPtr());
    if (retVal < 0) {
      vorbis_comment_clear(&vorbisComment);
      throw OggException("VorbisDecoder::initDecoder: packet is not a header");
    }

  }

  /* extract the comments*/
  for (uint8 i(0); i<vorbisComment.comments; ++i) {
    /* We have to extract the tags by ourself - there is no interface :-(*/
    std::string commentStr(vorbisComment.user_comments[i],
                           vorbisComment.comment_lengths[i]);

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

  /* finish initialization */
  vorbis_synthesis_init(&vorbisDspState,&vorbisInfo);
  vorbis_block_init(&vorbisDspState,&vorbisBlock);

  VorbisStreamParameter& vorbisConfig = dynamic_cast<VorbisStreamParameter&>(*config.parameter.get());

  vorbisConfig.samplerate = vorbisInfo.rate;
  vorbisConfig.channels   = vorbisInfo.channels;
  vorbisConfig.datarate   = vorbisInfo.bitrate_nominal;
  vorbisConfig.datarateMin = vorbisInfo.bitrate_lower;
  vorbisConfig.datarateMax = vorbisInfo.bitrate_upper;
  vorbisConfig.datarateWin = vorbisInfo.bitrate_window;

  /* set the state machine */
  setConfigured();

}

MediaOutputDecoder& VorbisDecoder::operator<<(OggPacket packet)
{
  if (!isConfigured()) {
    throw OggException("VorbisDecoder::operator<<: stream not configured");
  }

  if (vorbis_synthesis(&vorbisBlock,packet->getUnderlayingOggPacketPtr())==0)
    vorbis_synthesis_blockin(&vorbisDspState,&vorbisBlock);

  float** pcm;
  int samples;

  while ((samples=vorbis_synthesis_pcmout(&vorbisDspState,&pcm))>0) {
    AudioPacket apack(new AudioPacketInternal(pcm, samples, vorbisInfo.channels));
    packetList.push_back(apack);
    vorbis_synthesis_read(&vorbisDspState,samples);
  }


  /* has there not been a packet in the queue before */
  if (!packetList.empty()) {
    /* set the internal state */
    setAvailable();
  }

  /* count the audio packets, to have a glimps of the actual position */
  packetCount++;

  return(*this);
}

VorbisDecoder& VorbisDecoder::operator>>(AudioPacket& audioPacket)
{
  if (!isAvailable())
    throw OggException("VorbisDecoder::operator>>: No audio packets available");

  audioPacket = packetList.front();
  packetList.pop_front();

  sampleCounter += audioPacket->getLength();

//  logger.debug() << "  Sample counter: "<<sampleCounter;
  if (packetList.empty())
    setEmpty();

  return(*this);
}

vorbis_comment& VorbisDecoder::getComment()
{
  return (vorbisComment);
}

double VorbisDecoder::getTimeOfNextPacket()
{
  return (sampleCounter/vorbisInfo.rate);
}

vorbis_info& VorbisDecoder::getInfo()
{
  return(vorbisInfo);
}

std::string VorbisDecoder::configuration() const
{
  std::stringstream stream;

  stream << "Vorbis Decoder Configuration:" << std::endl;
  stream << std::endl;
  stream << "Vorbis Version   : " << vorbisInfo.version << std::endl;
  stream << std::endl;
  stream << "Channel Number   : " << vorbisInfo.channels << std::endl;
  stream << "Sample rate      : " << vorbisInfo.rate << std::endl;
  stream << std::endl;
  stream << "Bitrate (nominal): " << vorbisInfo.bitrate_nominal << std::endl;
  if ( vorbisInfo.bitrate_lower > 0 )
    stream << "Bitrate (lower)  : " << vorbisInfo.bitrate_lower << std::endl;
  if ( vorbisInfo.bitrate_upper > 0 )
    stream << "Bitrate (upper)  : " << vorbisInfo.bitrate_upper << std::endl;
  //stream << "Bitrate (window) : " << vorbisInfo.bitrate_window << std::endl;
  stream << std::endl;

  if (vorbisComment.comments) {
    stream << "Comments:\n";
    for (int i=0; i<vorbisComment.comments; ++i)
      stream << vorbisComment.user_comments[i] << std::endl;

    stream << std::endl;
  }

  return(stream.str());
}


#endif // WITH_LIBVORBIS
