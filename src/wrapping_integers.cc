#include "wrapping_integers.hh"


using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { (uint32_t)( n + zero_point.raw_value_ % ( 1ULL << 32 ) ) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{

  uint64_t r;

  //计算出offset。因为绝对值必然是新值在后，zero值在前。所以差值固定。
  if ( raw_value_ >= zero_point.raw_value_ ) {
    r = checkpoint + raw_value_ - zero_point.raw_value_;
  } else {
    r = checkpoint + ( 1ULL << 32 ) + raw_value_ - zero_point.raw_value_;
  }

  //增加checkpoint的值
  r += checkpoint;

  // 如果数据是在checkpoint之后，那么到这步就已经结束了，因为只需要加上checkpoint的值就可以了。
  // 但是由于数据有可能在checkpoint之前，所以需要额外校验。
  //
  // 1.有可能在checkpoint之前，所以要检验一下在checkpoint左边还是在右边，选择差值比较小的。
  // 2.在第一个checkpoint边界到达之前，没有左边
  if ( raw_value_ >= ( 1ULL << 31 ) && r >= ( 1ULL << 32 ) ) {
    r -= ( 1ULL << 32 );
  }

  return r;
}
