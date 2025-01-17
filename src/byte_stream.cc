#include "byte_stream.hh"

using namespace std;

bool Writer::is_closed() const
{
  return closed_;
}

void Writer::push( const string& data )
{
  if ( data.empty() ) {
    return;
  }

  string insertable_data = data.substr( 0, available_capacity() );

  if ( insertable_data.empty() ) {
    return;
  }

  buffer_.insert( buffer_.end(), insertable_data.begin(), insertable_data.end() );
  pushed_+= insertable_data.size();
}

void Writer::close()
{
  closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  uint64_t remain = capacity_ - buffer_.size();
  return remain;
}

uint64_t Writer::bytes_pushed() const
{
  return pushed_;
}

bool Reader::is_finished() const
{
  return closed_ && buffer_.empty();
}

uint64_t Reader::bytes_popped() const
{
  return popped_;
}

string_view Reader::peek() const
{
  return {&buffer_.front(), 1};
}

void Reader::pop( uint64_t len )
{
  for ( uint64_t i = 0; i < len; ++i ) {
    buffer_.pop_front();
    popped_++;
  }
}

uint64_t Reader::bytes_buffered() const
{
  return buffer_.size();
}
