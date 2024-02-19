#include "reassembler.hh"
#include <cassert>

using namespace std;

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

  assert( first_index >= next_index_ );
  if ( first_index == next_index_ ) {
    push_to_writer_stream( data, is_last_substring );
    next_index_ += data.size();
    check_and_write_from_internal();
  } else {
    insert_into_internal( first_index, data );
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
