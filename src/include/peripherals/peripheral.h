#ifndef __PERIPHERAL_H__
#define __PERIPHERAL_H__

#include <cstdint>
#include <iostream>
#include <string>

typedef uint64_t number;

using namespace std;

class RP2040;

class Peripheral {
public:
  virtual number readUint32(number offset) = 0;
  virtual void writeUint32(number offset, number value) = 0;
};

class LoggingPeripheral : public Peripheral {
public:
  RP2040 *rp2040;
  string name;

  LoggingPeripheral(RP2040 *rp2040, string name);

  number readUint32(number offset);
  void writeUint32(number offset, number value);
};

class UnimplementedPeripheral : public LoggingPeripheral {
public:
  UnimplementedPeripheral(RP2040 *rp2040, string name)
      : LoggingPeripheral(rp2040, name) {}
};

#endif