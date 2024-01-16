#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  return this->isClosed;
}

void Writer::push( string data )
{
  uint64_t count = 0;
  for(uint64_t i = 0; i < (uint64_t)data.length(); i++) {
    if (available_capacity() == 0 || this->isClosed) break;
    this->deck_.push_back(data[i]);
    count++;
  }

  this->totalPushed += count;
  return;
}

void Writer::close()
{
  this->isClosed = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - (uint64_t)this->deck_.size();
}

uint64_t Writer::bytes_pushed() const
{
  return totalPushed;
}

bool Reader::is_finished() const
{
  return this->isClosed && (this->deck_.size() == 0);
}

uint64_t Reader::bytes_popped() const
{
  return this->totalPopped;
}

string_view Reader::peek() const
{
  return string_view(&this->deck_.front(), 1);

  }

void Reader::pop( uint64_t len )
{
  for (uint64_t i = 0; i < len; i++) {
    if (this->deck_.size() == 0) break;
    this->deck_.pop_front();
    this->totalPopped++;
  }
}

uint64_t Reader::bytes_buffered() const
{
  return this->deck_.size();
}
