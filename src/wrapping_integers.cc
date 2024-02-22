#include "wrapping_integers.hh"
#include <cstdint>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { (uint32_t)( ( n + zero_point.raw_value_ ) % ( 1ULL << 32 ) ) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{

  uint64_t r;
  uint64_t offset;

  // 计算出offset。因为绝对值必然是新值在后，zero值在前。所以相对值的差值固定。
  offset = getAbs( zero_point.raw_value_, raw_value_ );

  uint64_t cycle_base = ( checkpoint / (1ULL << 32) ) * ( 1ULL << 32 );
  r = offset + cycle_base * ( 1ULL << 32 );

  // 如果offset大于2^31，那么r需要加上2^32
  if ( getAbs(r, checkpoint - cycle_base) >= ( 1ULL << 31 ) ) {
    r += ( 1ULL << 32 );
  }

  return r;
}

uint64_t Wrap32::getAbs( uint64_t a1, uint64_t a2 )
{
  uint64_t offset;
  if ( a2 >= a1 ) {
    offset = a2 - a1;
  } else {
    // 因为是一个循环，所以计算值时要谨慎
    offset = ( UINT32_MAX + 1 ) - ( a1 - a2 );
  }
  return offset;
}
