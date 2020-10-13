#ifndef THEORAHEADER_H
#define THEORAHEADER_H

struct TheoraHeader {
  char    vmaj;
  char    vmin;
  char    vrev;
  uint16  fmbw;
  uint16  fmbh;
  uint32  picw:24;
  uint32  pich:24;
  char    picx;
  char    picy;
  uint32  frn;
  uint32  frd;
  uint32  parn:24;
  uint32  pard:24;
  char    cs;
  uint32  nombr:24;

  // its little endian
  // and network byte order
  union {
    struct {
      uint16  reserved:3;
      uint16  pf:2;
      uint16  kfgshift:5;
      uint16  qual:6;
    } lenbo;
    uint16 pleaseconvert;
  } un;

} __attribute__ ((packed));

#endif
