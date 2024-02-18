#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // 校验是否应该写入数据（front_index == now_front_bound）
  uint64_t end_index = first_index + data.size() - 1;

  // 丢弃数据
  if ( end_index < next_index_ ) {
    return;
  }

  if ( beyond_capacity( first_index, data ) ) {
    data = data.substr( 0, next_index_ + output_.writer().available_capacity() - first_index);
  }

  insert_into_internal( first_index, data );
  check_and_write_from_internal();

  if ( is_last_substring ) {
    output_.writer().close();
  }

}

void Reassembler::insert_into_internal( uint64_t first_index, const string& data )
{
  uint64_t index = first_index;
  for ( const auto& item : data ){
    internal_storage[ index++ ] = item;
  }
}

void Reassembler::check_and_write_from_internal()
{
  string data;

  while (internal_storage.find(next_index_) != internal_storage.end()) {
    data.append(1, internal_storage[next_index_]);
    internal_storage.erase(next_index_++);
  }

  push_to_writer_stream(data);
}


bool Reassembler::beyond_capacity( uint64_t first_index, const string& data )
{
  uint64_t end_index = first_index + data.size() - 1;
  if ( end_index > next_index_ + output_.writer().available_capacity() ) {
    return true;
  }
  return false;
}

void Reassembler::push_to_writer_stream( const string& data )
{
  if ( data.empty() ) {
    return;
  }

  next_index_ += data.size();
  output_.writer().push( data );
}

uint64_t Reassembler::bytes_pending() const
{
  return internal_storage.size();
}
