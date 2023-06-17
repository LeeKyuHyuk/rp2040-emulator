#include "peripherals/peripheral.h"
#include "rp2040.h"
#include <iostream>

LoggingPeripheral::LoggingPeripheral(RP2040 *rp2040, string name) {
  this->rp2040 = rp2040;
  this->name = name;
}

number LoggingPeripheral::readUint32(number offset) {
  cout << "Unimplemented peripheral " << this->name << " read from 0x" << hex
       << offset << endl;
  if (offset > 0x1000) {
    cout << "Unimplemented read from peripheral in the atomic operation region"
         << endl;
  }
  return 0xffffffff;
}

void LoggingPeripheral::writeUint32(number offset, number value) {
  cout << "Unimplemented peripheral " << this->name << " write to 0x" << hex
       << offset << ": 0x" << hex << value << endl;
  if (offset > 0x1000) {
    cout << "Unimplemented atomic-write to peripheral " << this->name << endl;
  }
}
