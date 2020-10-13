#ifndef KATEHEADER_H_
#define KATEHEADER_H_

struct KateHeader {
  uint8 magic0;
  uint8 reserved0;
  uint8 versionMajor;
  uint8 versionMinor;
  uint8 numHeaders;
  uint8  textEncoding;
  uint8  textDirectionality;
  uint8  reserved1;
  uint8  granuleShift;
  uint16 canvasWidthShift:4;
  uint16 canvasWidthBase:12;
  uint16 canvasHeightShift:4;
  uint16 canvasHeightBase:12;
  uint32 reserved2;
  uint32 granulerateNumerator;
  uint32 granulerateDenominator;
  char   language[16];
  char   category[16];
} __attribute__ ((packed));


#endif /*KATEHEADER_H_*/
