#include "Bitmask.h"

void Bitmask::SetBit(uint32_t pos, bool on)
{
  if (on)
  {
    bits = bits | (1 << pos);
  }
  else
  {
    bits = bits & ~(1 << pos);
  }
}