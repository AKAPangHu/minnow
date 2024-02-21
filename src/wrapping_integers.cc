#include "wrapping_integers.hh"

using namespace std;


Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { (uint32_t) (n + zero_point.raw_value_ % (1ULL << 32))};
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t result;

  if (raw_value_ >= zero_point.raw_value_) {
    result = checkpoint + raw_value_ - zero_point.raw_value_;
  } else {
    result = checkpoint + (1ULL << 32) + raw_value_ - zero_point.raw_value_;
  }

  return result;
}
