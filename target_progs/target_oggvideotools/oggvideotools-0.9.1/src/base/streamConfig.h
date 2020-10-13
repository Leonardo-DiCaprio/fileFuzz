#ifndef STREAMCONFIG_H_
#define STREAMCONFIG_H_

#include <vector>

#include "oggPacket.h"
#include "streamExtractor.h"


/* this configuration gives all information, that the outside world needs
 * to know about the streams */
class StreamConfig  : public ExtractorInformation {

public:

  //! Id of the stream (starts from 0 and is incremented by every new stream)
  int8                   streamNo;

  //! Every stream needs some header packets, which are stored here
  /*! at least one "Begin of Stream" packet and maybe some additional packets. These
   *  will all be stored here */
  std::vector<OggPacket> headerList;

};

#endif
