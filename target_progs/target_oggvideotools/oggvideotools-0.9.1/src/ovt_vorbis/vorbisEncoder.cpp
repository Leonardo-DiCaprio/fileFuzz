#include "vorbisEncoder.h"

#ifdef HAVE_LIBVORBIS

#include "vorbisExtractor.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include "exception.h"
#include "log.h"

VorbisEncoder::VorbisEncoder(uint8 _streamNo) :
  MediaInputEncoder(_streamNo), pktCnt(0)
{
}

VorbisEncoder::~VorbisEncoder()
{

  if (isConfigured()) {
    vorbis_block_clear(&vorbisBlock);
    vorbis_dsp_clear(&vorbisState);
    vorbis_info_clear(&vorbisInfo);
  }

}

vorbis_info& VorbisEncoder::getInfo()
{
  return(vorbisInfo);
}

void VorbisEncoder::configureEncoder(StreamConfig& streamConfig, std::vector<OggComment>& oggComments)
{
  if (isConfigured())
    throw OggException("VorbisEncoder::setConfig: can't configure encoder twice");

  vorbis_info_init(&vorbisInfo);

  VorbisStreamParameter& config = dynamic_cast<VorbisStreamParameter&>(*streamConfig.parameter.get());

  /*	int32 ret = vorbis_encode_init(&vorbisInfo, config.channels,
   config.samplerate, config.datarate, config.datarate, config.datarate);
   */
  int32 ret = vorbis_encode_init(&vorbisInfo, config.channels,
                                 config.samplerate, -1, config.datarate, -1);

  /* do not continue if setup failed; this can happen if we ask for a
   mode that libVorbis does not support (eg, too low a bitrate, etc,
   will return 'OV_EIMPL') */

  if (ret)
    throw OggException("VorbisEncoder::configureEncoder: can not configure encoder, wrong parameters");

  /* add a comment */
  vorbis_comment_init(&vorbisComment);
  vorbis_comment_add_tag(&vorbisComment, "ENCODER", PACKAGE_STRING);

  /* add other comments */
  for (uint32 i(0); i<oggComments.size(); ++i)
    vorbis_comment_add_tag(&vorbisComment,
                           (char*) oggComments[i].tag.c_str(),
                           (char*) oggComments[i].value.c_str());

  /* set up the analysis state and auxiliary encoding storage */
  vorbis_analysis_init(&vorbisState, &vorbisInfo);
  vorbis_block_init(&vorbisState, &vorbisBlock);

  OggPacket header      = std::make_shared<OggPacketInternal>();
  OggPacket header_comm = std::make_shared<OggPacketInternal>();
  OggPacket header_code = std::make_shared<OggPacketInternal>();

  vorbis_analysis_headerout(&vorbisState, &vorbisComment,
                            header->getUnderlayingOggPacketPtr(),
                            header_comm->getUnderlayingOggPacketPtr(),
                            header_code->getUnderlayingOggPacketPtr() );

  header->setStreamType(OggType::vorbis);
  header->setStreamNo(streamNo);
  header->setStreamHeader();

#ifdef DEBUG
  logger.debug() << "Vorbis Packet Number: "<< header.packetno << "reset to 0" << std::endl;
#endif
  header->setPacketno(0);

  streamConfig.headerList.push_back(header);

  header_comm->setStreamType(OggType::vorbis);
  header_comm->setStreamNo(streamNo);
  header_comm->setStreamHeader();

#ifdef DEBUG
  logger.debug() << "Vorbis Packet Number: "<< header_comm.packetno << "reset to 0" << std::endl;
#endif
  header_comm->setPacketno(0);

  streamConfig.headerList.push_back(header_comm);

  header_code->setStreamType(OggType::vorbis);
  header_code->setStreamNo(streamNo);
  header_code->setStreamHeader();

#ifdef DEBUG
  logger.debug() << "Vorbis Packet Number: "<< header_code.packetno << "reset to 0" << std::endl;
#endif
  header_code->setPacketno(0);

  streamConfig.headerList.push_back(header_code);

  VorbisExtractor extractor;
  extractor.extract(streamConfig.headerList[0], streamConfig);

  streamConfig.numOfHeaderPackets = streamConfig.headerList.size();
//	streamConfig.parameter = new VorbisStreamParameter(config);
  streamConfig.type = OggType::vorbis;
  streamConfig.streamNo = streamNo;
  streamConfig.serialNo = rand();

  vorbis_comment_clear(&vorbisComment);

  setConfigured();

  // reset packet Counter, in case, the stream reconfigured
  pktCnt = 0;

}

MediaInputEncoder& VorbisEncoder::operator<<(AudioPacket& aPacket)
{
  float **buffer=vorbis_analysis_buffer(&vorbisState, aPacket->getLength());

  /* there is no chance to give the data directly to the encoder
   * so we need to copy :-( */
  for (uint8 i(0); i<vorbisInfo.channels; ++i) {
    memcpy(buffer[i], aPacket->getDataOfChannel(i), aPacket->getLength()*sizeof(float));
  }

  /* tell the library how much we actually submitted */
  if (vorbis_analysis_wrote(&vorbisState, aPacket->getLength()) < 0)
    throw OggException("VorbisEncoder::operator <<: Invalid value");

  /* vorbis does some data preanalysis, then divvies up blocks for
   more involved (potentially parallel) processing.  Get a single
   block for encoding now */
  while ((vorbis_analysis_blockout(&vorbisState, &vorbisBlock))==1) {

    //OggPacket packet = std::make_shared<OggPacketInternal>();
    ogg_packet tmpPkt;
    /* analysis, assume we want to use bitrate management */
    vorbis_analysis(&vorbisBlock,0);
    vorbis_bitrate_addblock(&vorbisBlock);

    while (vorbis_bitrate_flushpacket(&vorbisState, &tmpPkt/*packet->getUnderlayingOggPacketPtr()*/)) {
//			logger.debug() << "Position: "<<packet.granulepos<<std::endl;

      //    ost::slog(ost::Slog::levelDebug) << "TheoraEncoder:: inserting header/n";
      OggPacket packet = std::make_shared<OggPacketInternal>(tmpPkt);
      packet->setStreamType(OggType::vorbis);
      packet->setStreamNo(streamNo);
#ifdef DEBUG
      logger.debug() << "Vorbis Packet Number: "<< packet->getPacketno() << std::endl;
#endif
      packet->setPacketno(pktCnt++);
      packetList.push_back(packet);
    }
  }

  if (!packetList.empty())
    setAvailable();

  return(*this);
}

MediaInputEncoder& VorbisEncoder::operator >>(OggPacket& packet)
{
  if (packetList.empty())
    throw OggException("VorbisEncoder::operator>> PacketList is m_empty");

  packet = packetList.front();
  packetList.pop_front();

  if (packetList.empty())
    setEmpty();

  return(*this);
}

void VorbisEncoder::flush()
{
  /* tell the library how much we actually submitted */
  if (vorbis_analysis_wrote(&vorbisState, 0) < 0)
    throw OggException("VorbisEncoder::flush: can not flush");

  /* vorbis does some data preanalysis, then divvies up blocks for
   more involved (potentially parallel) processing.  Get a single
   block for encoding now */
  while ((vorbis_analysis_blockout(&vorbisState, &vorbisBlock))==1) {

    /* analysis, assume we want to use bitrate management */
    vorbis_analysis(&vorbisBlock,0);
    vorbis_bitrate_addblock(&vorbisBlock);

    bool do_flush(true);

    while ( do_flush ) {
//      logger.debug() << "Flush: "<<packet.granulepos<<std::endl;
      ogg_packet tmpPkt;
      do_flush = vorbis_bitrate_flushpacket(&vorbisState, &tmpPkt) == 1;

      //    ost::slog(ost::Slog::levelDebug) << "TheoraEncoder:: inserting header/n";
      OggPacket packet = std::make_shared<OggPacketInternal>(tmpPkt);

      packet->setStreamType(OggType::vorbis);
      packet->setStreamNo(streamNo);
      packet->setPacketno(pktCnt++);

      packetList.push_back(packet);
    }
  }

  if (!packetList.empty())
    setAvailable();

}

std::string VorbisEncoder::configuration() const
{
  std::stringstream stream;

  stream << "Vorbis Encoder Configuration:" << std::endl;
  stream << "Stream No: "<<(int)streamNo<<std::endl;
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
//	stream << "Bitrate (window) : " << vorbisInfo.bitrate_window << std::endl;
  stream << std::endl;

  if (vorbisComment.comments) {
    stream << "Comments:\n";
    for (int i=0; i<vorbisComment.comments; ++i)
      stream << vorbisComment.user_comments[i] << std::endl;

    stream << std::endl;
  }
  return(stream.str());
}

#endif
