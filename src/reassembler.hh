#pragma once

#include "byte_stream.hh"
#include <cstdint>
#include <cstdio>
#include <sys/types.h>
#include <tuple>
#include <vector>

class Reassembler
{
public:
  // Construct Reassembler to write into given ByteStream.
  explicit Reassembler( ByteStream&& output ) : output_( std::move( output ) ) {}

  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;

  // Access output stream reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // Access output stream writer, but const-only (can't write from outside)
  const Writer& writer() const { return output_.writer(); }

private:
  ByteStream output_; // the Reassembler writes to this ByteStream
  uint64_t storage_capacity = output_.getCapacity();
  uint64_t cur_index = 0;
  uint64_t final_index = output_.getCapacity() - 1;
  uint64_t total_len = 0;
  bool seen_last = false;
  std::string storage = "";
  std::string storage_bitmap = "";

  bool is_valid_index( uint64_t index )
  {
    if ( this->writer().bytes_pushed() < storage_capacity ) {
      return ( ( index >= cur_index ) && ( index < cur_index + storage_capacity ) );
    } else {

      uint64_t new_index = index % storage_capacity;
      return ( ( new_index >= cur_index % storage_capacity )
               && ( new_index < ( cur_index % storage_capacity ) + storage_capacity ) );
    }
  }

  uint64_t last_index() { return final_index; }
};
