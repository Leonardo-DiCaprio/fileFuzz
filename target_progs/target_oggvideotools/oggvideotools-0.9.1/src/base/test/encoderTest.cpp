//
// Created by joern on 23.07.16.
//

#include "oggStreamEncoder.h"
#include <vector>
#include <iostream>

void printif(OggPage page)
{
  if (page) {
    std::cout << page->toString(5);
  } else {
    std::cout << "page is empty"<<std::endl;
  }
}

int main(int argc, char* argv[])
{

  unsigned char data[] = "hallo";
  uint32_t length = 6;

  OggPacket packet = OggPacketInternal::create(data, length,0,0);
  OggPage page;

  OggStreamEncoder encoder;

  packet->setBOS();

  encoder << packet;

  std::vector<uint8_t> packetData2;
  packetData2.insert(packetData2.end(),254, 0x5c);

  packet = OggPacketInternal::create(&packetData2[0], packetData2.size(),0,0);
  page.reset();

  encoder << packet;

  while (!encoder.isEmpty()) {
    page.reset();
    encoder >> page;

    printif(page);
  }

  std::vector<uint8_t> packetData3;
  packetData3.insert(packetData3.end(),1276, 0xb4);

  packet = OggPacketInternal::create(&packetData3[0], packetData3.size(),0,0);
  page.reset();

  encoder << packet;
  encoder.flush();

  while (!encoder.isEmpty()) {
    page.reset();
    encoder >> page;

    printif(page);
  }


  return 0;

}
