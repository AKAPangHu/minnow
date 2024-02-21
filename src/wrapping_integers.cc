#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { (uint32_t)( n + zero_point.raw_value_ % ( 1ULL << 32 ) ) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t r;
  if ( raw_value_ >= zero_point.raw_value_ ) {
    r = checkpoint + raw_value_ - zero_point.raw_value_;
  } else {
    r = checkpoint + ( 1ULL << 32 ) + raw_value_ - zero_point.raw_value_;
  }

  if ( checkpoint == 0 ) {
    return r;
  }

  if ( r == checkpoint ) {
    return r;
  } else if ( r > checkpoint ) {
    return r + 1;
  } else {
    return r - 1;
  }
}
