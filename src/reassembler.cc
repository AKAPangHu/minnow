#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{

  // 校验是否应该写入数据（front_index == now_front_bound）
  if ( next_index_ == first_index ) {
    output_.writer().push( data );
    next_index_ += data.size();
    checkAndWriteBuffer();
  } else if ( next_index_ < first_index ) {
    findAndInsertIntoBuffer( first_index, std::move( data ) );
  } else {
    // 丢弃数据
  }

  if ( is_last_substring ) {
    output_.writer().close();
  }
  // 读取缓冲区的时机：
  // 1、刚好在上面写入完数据，bound向后推移。循环遍历缓冲区，直到不符合顺序为止。

  // delete:引入竞争条件，需要一个简单的锁，保证不重复发送相同的数据包。
}
void Reassembler::findAndInsertIntoBuffer( uint64_t first_index, string&& data )
{ // 顺序查找位置，如果重复，则丢弃数据
  BufferItem item( first_index, data );
  bool inserted = false;

  for ( auto it = buffer_.begin(); it != buffer_.end(); ++it ) {
    if ( it->index_ == first_index ) {
      inserted = true;
      continue;
    }

    if ( it->index_ > first_index ) {
      buffer_.insert( it, item );
      inserted = true;
      break;
    }
  }

  // 如果没有找到合适的位置，则代表缓冲区为空，或者index是最大的。插入到末尾。
  if ( !inserted ) {
    buffer_.push_back( item );
  }
}

void Reassembler::checkAndWriteBuffer()
{
  while ( next_index_ == buffer_.front().index_ ) {
    output_.writer().push( buffer_.front().data_ );
    next_index_ += buffer_.front().data_.size();
    buffer_.pop_front();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return buffer_.size();
}
