#include "byte_stream.hh"

using namespace std;

bool Writer::is_closed() const
{
  return closed_;
}

void Writer::push( string data )
{
  for ( auto c : data ) {
    if ( buffer_.size() >= capacity_ ) {
      return;
    }

    buffer_.push_back( c );
    pushed_++;
  }
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
