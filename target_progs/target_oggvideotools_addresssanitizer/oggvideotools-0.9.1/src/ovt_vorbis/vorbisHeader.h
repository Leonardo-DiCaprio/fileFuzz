#ifndef VORBISHEADER_H_
#define VORBISHEADER_H_

struct VorbisHeader {
  uint32 version;
  uint8  audioChannels;
  uint32 sampleRate;
  uint32 bitrateMax;
  uint32 bitrateNom;
  uint32 bitrateMin;
  uint8  blocksize0:4;
  uint8  blocksize1:4;
  uint8  framing; //?
} __attribute__ ((packed));


#endif /*VORBISHEADER_H_*/
