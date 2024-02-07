#include "byte_stream.hh"
#include <cstdint>
#include <string>
#include <string_view>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  return this->isClosed;
}

void Writer::push( string data )
{

  if ( available_capacity() == 0 || this->isClosed || data == "" )
    return;

  string cur_str = data.substr( 0, available_capacity() );
  this->deck_.push_back( cur_str );

  this->totalPushed += cur_str.length();
  bytes_in_deck_ += cur_str.length();
}

void Writer::close()
{
  this->isClosed = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - bytes_in_deck_;
}

uint64_t Writer::bytes_pushed() const
{
  return totalPushed;
}

bool Reader::is_finished() const
{
  return this->isClosed && ( this->deck_.size() == 0 );
}

uint64_t Reader::bytes_popped() const
{
  return this->totalPopped;
}

string_view Reader::peek() const
{
  if ( this->deck_.empty() )
    return "";

  return this->deck_.front();
}

void Reader::pop( uint64_t len )
{
  if ( this->deck_.size() == 0 )
    return;

  uint64_t bytes_removed = 0;
  while ( true ) {
    if ( bytes_removed == len )
      break;

    string& cur_str = this->deck_.front();
    if ( cur_str.length() <= len - bytes_removed ) {
      bytes_removed += cur_str.length();
      bytes_in_deck_ -= cur_str.length();
      this->totalPopped += cur_str.length();
      this->deck_.pop_front();
    } else {
      bytes_in_deck_ -= cur_str.length();

      uint64_t prev_len = cur_str.length();
      cur_str = cur_str.substr( len - bytes_removed, cur_str.length() );
      bytes_removed += ( prev_len - cur_str.length() );
      this->totalPopped += ( prev_len - cur_str.length() );
      bytes_in_deck_ += cur_str.length();
    }
  }
}

uint64_t Reader::bytes_buffered() const
{
  return bytes_in_deck_;
}
