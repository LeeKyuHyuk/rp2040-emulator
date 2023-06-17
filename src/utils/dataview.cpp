#include "utils/dataview.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

DataView::DataView(uint8_t buffer[], uint64_t length) {
  this->type = UINT8_ARRAY;
  this->uint8Buffer = buffer;
  this->length = length;
}

DataView::DataView(uint32_t buffer[], uint64_t length) {
  this->type = UINT32_ARRAY;
  this->uint32Buffer = buffer;
  this->length = length;
}

uint8_t DataView::getUint8(uint64_t byteOffset) {
  DATAVIEW_TYPE type = this->type;
  uint32_t value = 0xFFFFFFFE;

  switch (type) {
  case UINT8_ARRAY: {
    uint8_t *array = (uint8_t *)this->uint8Buffer;
    memcpy(&value, &array[byteOffset], sizeof(uint8_t));
    break;
  }

  case UINT16_ARRAY: {
    uint16_t *array = (uint16_t *)this->uint16Buffer;
    uint8_t *byteArray = (uint8_t *)malloc(sizeof(uint16_t) * (this->length));
    memcpy(byteArray, array, sizeof(uint16_t) * (this->length));
    memcpy(&value, &byteArray[byteOffset], sizeof(uint8_t));
    free(byteArray);
    break;
  }

  case UINT32_ARRAY: {
    uint32_t *array = (uint32_t *)this->uint32Buffer;
    uint8_t *byteArray = (uint8_t *)malloc(sizeof(uint32_t) * (this->length));
    memcpy(byteArray, array, sizeof(uint32_t) * (this->length));
    memcpy(&value, &byteArray[byteOffset], sizeof(uint8_t));
    free(byteArray);
    break;
  }
  }

  return value;
}

void DataView::setUint8(uint64_t byteOffset, uint8_t value) {
  switch (this->type) {
  case UINT8_ARRAY: {
    uint8_t *array = (uint8_t *)this->uint8Buffer;
    memcpy(&array[byteOffset], &value, sizeof(uint8_t));
    break;
  }

  case UINT16_ARRAY: {
    uint16_t *array = (uint16_t *)this->uint16Buffer;
    uint8_t *byteArray = (uint8_t *)malloc(sizeof(uint16_t) * (this->length));
    memcpy(byteArray, array, sizeof(uint16_t) * (this->length));
    memcpy(&byteArray[byteOffset], &value, sizeof(uint8_t));
    memcpy(array, &byteArray[0], sizeof(uint16_t) * (this->length));
    free(byteArray);
    break;
  }

  case UINT32_ARRAY: {
    uint32_t *array = (uint32_t *)this->uint32Buffer;
    uint8_t *byteArray = (uint8_t *)malloc(sizeof(uint32_t) * (this->length));
    memcpy(byteArray, array, sizeof(uint32_t) * (this->length));
    memcpy(&byteArray[byteOffset], &value, sizeof(uint8_t));
    memcpy(array, &byteArray[0], sizeof(uint32_t) * (this->length));
    free(byteArray);
    break;
  }
  }
}

uint16_t DataView::getUint16(uint64_t byteOffset) {
  DATAVIEW_TYPE type = this->type;
  uint32_t value = 0xFFFFFFFE;

  switch (type) {
  case UINT8_ARRAY: {
    uint8_t *array = (uint8_t *)this->uint8Buffer;
    memcpy(&value, &array[byteOffset], sizeof(uint16_t));
    break;
  }

  case UINT16_ARRAY: {
    uint16_t *array = (uint16_t *)this->uint16Buffer;
    uint8_t *byteArray = (uint8_t *)malloc(sizeof(uint16_t) * (this->length));
    memcpy(byteArray, array, sizeof(uint16_t) * (this->length));
    memcpy(&value, &byteArray[byteOffset], sizeof(uint16_t));
    free(byteArray);
    break;
  }

  case UINT32_ARRAY: {
    uint32_t *array = (uint32_t *)this->uint32Buffer;
    uint8_t *byteArray = (uint8_t *)malloc(sizeof(uint32_t) * (this->length));
    memcpy(byteArray, array, sizeof(uint32_t) * (this->length));
    memcpy(&value, &byteArray[byteOffset], sizeof(uint16_t));
    free(byteArray);
    break;
  }
  }

  return value;
}

void DataView::setUint16(uint64_t byteOffset, uint16_t value) {
  switch (this->type) {
  case UINT8_ARRAY: {
    uint8_t *array = (uint8_t *)this->uint8Buffer;
    memcpy(&array[byteOffset], &value, sizeof(uint16_t));
    break;
  }

  case UINT16_ARRAY: {
    uint16_t *array = (uint16_t *)this->uint16Buffer;
    uint8_t *byteArray = (uint8_t *)malloc(sizeof(uint16_t) * (this->length));
    memcpy(byteArray, array, sizeof(uint16_t) * (this->length));
    memcpy(&byteArray[byteOffset], &value, sizeof(uint16_t));
    memcpy(array, &byteArray[0], sizeof(uint16_t) * (this->length));
    free(byteArray);
    break;
  }

  case UINT32_ARRAY: {
    uint32_t *array = (uint32_t *)this->uint32Buffer;
    uint8_t *byteArray = (uint8_t *)malloc(sizeof(uint32_t) * (this->length));
    memcpy(byteArray, array, sizeof(uint32_t) * (this->length));
    memcpy(&byteArray[byteOffset], &value, sizeof(uint16_t));
    memcpy(array, &byteArray[0], sizeof(uint32_t) * (this->length));
    free(byteArray);
    break;
  }
  }
}

uint32_t DataView::getUint32(uint64_t byteOffset) {
  DATAVIEW_TYPE type = this->type;
  uint32_t value = 0xFFFFFFFE;

  switch (type) {
  case UINT8_ARRAY: {
    uint8_t *array = (uint8_t *)this->uint8Buffer;
    memcpy(&value, &array[byteOffset], sizeof(uint32_t));
    break;
  }

  case UINT16_ARRAY: {
    uint16_t *array = (uint16_t *)this->uint16Buffer;
    uint8_t *byteArray = (uint8_t *)malloc(sizeof(uint16_t) * (this->length));
    memcpy(byteArray, array, sizeof(uint16_t) * (this->length));
    memcpy(&value, &byteArray[byteOffset], sizeof(uint32_t));
    free(byteArray);
    break;
  }

  case UINT32_ARRAY: {
    uint32_t *array = (uint32_t *)this->uint32Buffer;
    uint8_t *byteArray = (uint8_t *)malloc(sizeof(uint32_t) * (this->length));
    memcpy(byteArray, array, sizeof(uint32_t) * (this->length));
    memcpy(&value, &byteArray[byteOffset], sizeof(uint32_t));
    free(byteArray);
    break;
  }
  }

  return value;
}

void DataView::setUint32(uint64_t byteOffset, uint32_t value) {
  switch (this->type) {
  case UINT8_ARRAY: {
    uint8_t *array = (uint8_t *)this->uint8Buffer;
    memcpy(&array[byteOffset], &value, sizeof(uint32_t));
    break;
  }

  case UINT16_ARRAY: {
    uint16_t *array = (uint16_t *)this->uint16Buffer;
    uint8_t *byteArray = (uint8_t *)malloc(sizeof(uint16_t) * (this->length));
    memcpy(byteArray, array, sizeof(uint16_t) * (this->length));
    memcpy(&byteArray[byteOffset], &value, sizeof(uint32_t));
    memcpy(array, &byteArray[0], sizeof(uint16_t) * (this->length));
    free(byteArray);
    break;
  }

  case UINT32_ARRAY: {
    uint32_t *array = (uint32_t *)this->uint32Buffer;
    uint8_t *byteArray = (uint8_t *)malloc(sizeof(uint32_t) * (this->length));
    memcpy(byteArray, array, sizeof(uint32_t) * (this->length));
    memcpy(&byteArray[byteOffset], &value, sizeof(uint32_t));
    memcpy(array, &byteArray[0], sizeof(uint32_t) * (this->length));
    free(byteArray);
    break;
  }
  }
}
