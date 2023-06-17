#include "utils/time.h"
#include <cstddef>
#include <sys/time.h>

number getCurrentMicroseconds() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * (uint64_t)1000000) + tv.tv_usec;
}