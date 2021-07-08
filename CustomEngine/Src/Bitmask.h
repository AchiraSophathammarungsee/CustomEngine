
#ifndef BITMASK_H
#define BITMASK_H

#include <stdint.h>

class Bitmask
{
public:
  Bitmask() : bits(0) {};
  Bitmask(uint32_t bit) : bits(bit) {};

  Bitmask& operator=(const Bitmask& rhs) { bits = rhs.bits; return *this; };
  bool operator==(const Bitmask& rhs) { return bits == rhs.bits; };

  void SetMask(const Bitmask& other) { bits = other.bits; };
  uint32_t GetMask() const { return bits; };

  bool GetBit(uint32_t pos) const { return bits & (1 << pos); };
  void SetBit(uint32_t pos, bool on);

  void Clear() { bits = 0; };

private:
  uint32_t bits;
};

#endif