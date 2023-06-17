#ifndef __DATA_VIEW_H__
#define __DATA_VIEW_H__

#include <cstdint>

#define SWAP16(s) (((((s)&0xff) << 8) | (((s) >> 8) & 0xff)))

#define SWAP32(l)                                                              \
  (((((l)&0xff000000) >> 24) | (((l)&0x00ff0000) >> 8) |                       \
    (((l)&0x0000ff00) << 8) | (((l)&0x000000ff) << 24)))

enum DATAVIEW_TYPE { UINT8_ARRAY, UINT16_ARRAY, UINT32_ARRAY };

class DataView {
private:
  uint8_t *uint8Buffer;
  uint16_t *uint16Buffer;
  uint32_t *uint32Buffer;
  uint64_t length;
  DATAVIEW_TYPE type;

public:
  DataView(uint8_t buffer[], uint64_t length);
  DataView(uint32_t buffer[], uint64_t length);

  uint8_t getUint8(uint64_t byteOffset);
  void setUint8(uint64_t byteOffset, uint8_t value);
  uint16_t getUint16(uint64_t byteOffset);
  void setUint16(uint64_t byteOffset, uint16_t value);
  uint32_t getUint32(uint64_t byteOffset);
  void setUint32(uint64_t byteOffset, uint32_t value);
};

#endif