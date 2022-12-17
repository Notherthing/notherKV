#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <stdint.h>

#include <ctime>
#include <random>

namespace notherkv {
class Random {
 public:
 Random(uint32_t seed);
 private:
 uint32_t seed_ = 0;
 
};
}  // namespace notherkv

#endif