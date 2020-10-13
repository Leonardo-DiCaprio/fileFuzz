#ifndef STREAMEXTRACTOR_H_
#define STREAMEXTRACTOR_H_

#include "oggPage.h"
#include "oggPacket.h"
#include "streamParameter.h"
#include "granulePosInterpreter.h"

//! This is a baseclass for the configuration of a stream
/*! This class can be derived by more specific stream information
 * For the decoding process, parameter is created while the decoder
 * is initialized
 * For the encoding process, the parameter pointer must carry a
 * valid parameter list.
 * The parameter list is always owned by the ExtractorInformation class
 * and will be deleted when the ExtractorInformation object destructor is
 * called */

class ExtractorInformation {

public:

  ExtractorInformation();
  ExtractorInformation(const ExtractorInformation& extractorInfo);
  ~ExtractorInformation();

  //! Type of stream (e.g. ogg_vorbis, ogg_theora, ogg_speex)
  OggType                type;

  //! stream serial number (random number required by ogg)
  uint32                 serialNo;

  //! The first page/packet gives detailed information of the stream
  std::shared_ptr<StreamParameter>       parameter;

  //! the number of header packets must be identified by the stream type
  uint8                  numOfHeaderPackets;

  ExtractorInformation& operator=(const ExtractorInformation& extractorInfo);

};


class StreamExtractor {
public:
  StreamExtractor();

  virtual ~StreamExtractor();

  virtual bool extract(OggPage& page, ExtractorInformation& information) = 0;
  virtual bool extract(OggPacket& packet, ExtractorInformation& information) = 0;

};

#endif /*STREAMEXTRACTOR_H_*/
