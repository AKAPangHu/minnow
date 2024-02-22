#include "wrapping_integers.hh"
#include <cstdint>


using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { (uint32_t)( n + zero_point.raw_value_ % ( 1ULL << 32 ) ) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{

  uint64_t r;
  uint64_t offset;


  //计算出offset。因为绝对值必然是新值在后，zero值在前。所以差值固定。
  if ( raw_value_ >= zero_point.raw_value_ ) {
    offset =  raw_value_ - zero_point.raw_value_;
  } else {
    offset = ( UINT32_MAX + 1) - (zero_point.raw_value_ - raw_value_);
  }

  //增加checkpoint的值
  r = offset + checkpoint;

  // 如果数据规定是在checkpoint之后，那么到这步就已经结束了，因为只需要加上checkpoint的值就可以了。
  // 但是由于数据有可能在checkpoint之前，所以需要额外校验。又因为绝对值不是在上一循环，就是在下一循环，所以直接减去一个循环的值的就可以了。
  //
  // 1.如果offset + 0 > 二分之一循环最大值，这样的值不可能在A+1循环中离上一个checkpoint点是最近的。所以可以反向证明出来不符合条件的值属于上一循环。
  // 2.第一个循环不适用上一规则
  if ( offset >= ( 1ULL << 31 ) && r >= ( UINT32_MAX ) ) {
    r -= ( 1ULL << 32 );
  }

  return r;
}
