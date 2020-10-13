//
// Created by joern on 23.07.16.
//

#include "oggDecoder.h"
#include <vector>
#include <iostream>
#include <string.h>
int main(int argc, char* argv[])
{

  OggDecoder decoder;

  {
    // create a packet, that is valid in the eyes of the decoder
    std::vector<uint8_t> packet;

    OggHeader header;
    memset(&header, 0, sizeof(OggHeader));

    header.ogg[0] = 'O';
    header.ogg[1] = 'g';
    header.ogg[2] = 'g';
    header.ogg[3] = 'S';

    header.tableSegments = 2;
    uint8_t* hdr_ptr = (uint8_t*)&header;
    packet.insert(packet.end(), &hdr_ptr[0], &hdr_ptr[sizeof(OggHeader)]);

    /* add the segments table */
    packet.push_back(255);
    packet.push_back(1);

    /* add all the data (that is 0xc5) */
    for (uint32_t i(0); i < 256; ++i) {
      packet.push_back(0xc5);
    }

    RawMediaPacket rmp = createRawMediaPacket();
    rmp->setData(packet, false);
    decoder << rmp;
  }

  OggPage page;

  decoder >> page;

  std::vector<uint8_t> pg_data = page->data();
  uint32_t cnt;

  for (auto i : pg_data) {
    std::cout << " 0x"<<std::hex<<(int)i;
    if (cnt++%16 == 0)
      std::cout << std::endl;
  }


}
