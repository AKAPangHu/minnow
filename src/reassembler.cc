#include "reassembler.hh"

using namespace std;

/**
 * 实现的基本思路：
 * 1. 检查是否有立即结束流的数据，如果有则直接结束流。
 * 2. 检查是否超出容量，如果有则截断。
 * 3. 检查是否有重叠，如果有则截断
 * 4. 检查是否可以立即发送，如果有则立即发送。（同时清理内存 + 检查内存后边的数据）
 * 5. 不能立即发送则代表需要缓存，将数据插入到内部存储中。
 */
void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{

  // 恰好是准备结束的最后一个字符串（防止超出无符号整形范围）
  if ( is_last_substring && data.empty() && first_index == next_index_ ) {
    push_to_writer_stream( data, is_last_substring );
    return;
  }

  uint64_t end_index;

  if ( data.empty() ) {
    end_index = first_index;
  } else {
    end_index = first_index + data.size() - 1;
  }

  // 抛弃已经接收到的部分
  if ( end_index < next_index_ ) {
    return;
  }

  // 重叠部分处理
  if ( first_index < next_index_ && end_index >= next_index_ ) {
    data = data.substr( next_index_ - first_index );
    first_index = next_index_;
  }

  // 超出容量处理
  if ( beyond_capacity( end_index ) ) {
    data = data.substr( 0, next_index_ + output_.writer().available_capacity() - first_index );
  }

  if ( is_last_substring ) {
    if ( data.empty() ) {
      last_index = first_index - 1;
    } else {
      last_index = end_index;
    }
  }

  // 如果是符合条件的应该立即发送
  if ( first_index == next_index_ ) {
    push_to_writer_stream( data, is_last_substring );
    next_index_ += data.size();
    erase_between( first_index, next_index_ );
    check_and_write_from_internal();
  } else {
    insert_into_internal( first_index, data );
  }
}

void Reassembler::erase_between( uint64_t first_index, uint64_t next_index )
{
  for ( uint64_t i = first_index; i < next_index; i++ ) {
    internal_storage.erase( i );
  }
}

void Reassembler::insert_into_internal( uint64_t first_index, const string& data )
{
  uint64_t index = first_index;
  for ( const auto& item : data ) {
    internal_storage[index] = item;
    index++;
  }
}

void Reassembler::check_and_write_from_internal()
{
  string data;

  while ( internal_storage.find( next_index_ ) != internal_storage.end() ) {
    data.append( 1, internal_storage[next_index_] );
    internal_storage.erase( next_index_ );
    next_index_++;
  }

  bool is_last_substring = ( next_index_ > last_index );
  push_to_writer_stream( data, is_last_substring );
}

bool Reassembler::beyond_capacity( uint64_t end_index )
{
  if ( end_index >= next_index_ + output_.writer().available_capacity() ) {
    return true;
  }
  return false;
}

void Reassembler::push_to_writer_stream( const string& data, bool is_last_substring )
{
  if ( !data.empty() ) {
    output_.writer().push( data );
  }

  if ( is_last_substring ) {
    output_.writer().close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return internal_storage.size();
}
