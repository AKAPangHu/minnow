#include "wrapping_integers.hh"

using namespace std;


Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { (uint32_t) (n + zero_point.raw_value_ % (1ULL << 32))};
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  return checkpoint + raw_value_ - zero_point.raw_value_;
}
