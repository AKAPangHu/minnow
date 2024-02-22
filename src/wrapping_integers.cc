#include "wrapping_integers.hh"
#include <cstdint>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { (uint32_t)( ( n + zero_point.raw_value_ ) % ( 1ULL << 32 ) ) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{

  uint64_t r1, r2, r3;
  uint64_t delta1, delta2, delta3;
  uint64_t offset;

  // 计算出offset。因为绝对值必然是新值在后，zero值在前。所以相对值的差值固定。
  offset = getRelativeDistance( zero_point.raw_value_, raw_value_ );

  // 计算出cycle_base，也就是checkpoint所在的循环的起始位置(即当前循环的isn绝对起始位置)
  uint64_t cycle_base = ( checkpoint / ( 1ULL << 32 ) ) * ( 1ULL << 32 );
  r1 = offset + cycle_base - ( 1ULL << 32 );
  r2 = offset + cycle_base;
  r3 = offset + cycle_base + ( 1ULL << 32 );

  delta1 = getAbs( r1, checkpoint );
  delta2 = getAbs( r2, checkpoint );
  delta3 = getAbs( r3, checkpoint );

  if ( delta1 <= delta2 && delta1 <= delta3 ) {
    return r1;
  } else if ( delta2 <= delta1 && delta2 <= delta3 ) {
    return r2;
  } else {
    return r3;
  }

  return r1 - checkpoint < r2 - checkpoint ? r1 : r2;

  //  // 比较相对位置。看看计算的循环起始位置正不正确。如果offset大于等于2^31，则证明在下一循环中，那么r需要加上2^32
  //  if ( getRelativeDistance(offset, checkpoint - cycle_base) >= ( 1ULL << 31 ) ) {
  //    r += ( 1ULL << 32 );
  //  }
  //
  //  return r;
}

uint32_t Wrap32::getRelativeDistance( uint32_t a1, uint32_t a2 )
{
  uint32_t offset;
  if ( a2 >= a1 ) {
    offset = a2 - a1;
  } else {
    // 因为是一个循环，所以计算值时要谨慎
    offset = ( UINT32_MAX + 1 ) - ( a1 - a2 );
  }
  return offset;
}
uint64_t Wrap32::getAbs( uint64_t a1, uint64_t a2 )
{
  if ( a1 >= a2 ) {
    return a1 - a2;
  } else {
    return a2 - a1;
  }
}
