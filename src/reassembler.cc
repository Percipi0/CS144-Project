#include "reassembler.hh"
#include "byte_stream.hh"
#include <cstdint>
#include <sys/types.h>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if ( storage == "" && storage_bitmap == "" ) {
    std::string s( (int)storage_capacity, '0' );
    storage = s;
    storage_bitmap = s;
  }

  if ( first_index == cur_index ) {
    string temp_str = data.substr( 0, output_.writer().available_capacity() );
    output_.writer().push( temp_str );
    cur_index += temp_str.length();
    // final_index += temp_str.length();
  } else if ( first_index < cur_index ) {
    for ( uint64_t i = 0; i < data.length(); i++ ) {
      if ( ( first_index + i ) == cur_index ) {
        // push as much as possible!
        string temp_str = data.substr( i, this->output_.writer().available_capacity() );
        this->output_.writer().push( temp_str );
        cur_index += temp_str.length();
        // final_index += temp_str.length();
        break;
      }
    }
  } else {
    // first_index must be greater than cur_index

    // check if we've put an elem at the edge of the buffer, and whether we're trying to go beyond that
    if ( !is_valid_index( first_index ) ) {
      return;
    };

    if ( this->writer().bytes_pushed() < storage_capacity ) {
      string cur_str = data.substr( 0, last_index() - first_index + 1 );
      string ones( cur_str.length(), '1' );
      storage.replace( first_index, cur_str.length(), cur_str );
      storage_bitmap.replace( first_index, cur_str.length(), ones );
    } else {
      string cur_str = data.substr( 0, last_index() - ( first_index % storage_capacity + 1 ) );
      string ones( cur_str.length(), '1' );
      storage.replace( first_index % storage_capacity, cur_str.length(), cur_str );
      storage_bitmap.replace( first_index % storage_capacity, cur_str.length(), ones );
    }
  }

  // check if it's time to start pushing bytes in storage

  uint64_t new_cur_index = cur_index;
  if ( this->writer().bytes_pushed() > storage_capacity ) {
    new_cur_index %= storage_capacity;
  }

  if ( storage_bitmap[new_cur_index] == '1' ) {
    uint64_t count = 1;
    for ( uint64_t i = new_cur_index + 1; i < storage_bitmap.length(); i++ ) {
      if ( storage_bitmap[i] == '0' ) {
        break;
      }
      count++;
    }
    string cur_str = storage.substr( new_cur_index, count );
    this->output_.writer().push( cur_str );
    string zeros( count, '0' );
    storage_bitmap.replace( new_cur_index, zeros.length(), zeros );

    cur_index += cur_str.length();
    // final_index += cur_str.length();
  }

  // clean up unneeded bits
  string zeros( new_cur_index, '0' );
  storage_bitmap.replace( 0, zeros.length(), zeros );

  if ( is_last_substring ) {
    seen_last = true;
    total_len = first_index + data.length();
    /*if ( data.length() != 0 ) {
      final_index = first_index + data.length() - 1;
    }*/
  }

  if ( seen_last && this->writer().bytes_pushed() == total_len ) {
    this->output_.writer().close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  uint64_t count = 0;
  for ( uint64_t i = 0; i < storage_bitmap.length(); i++ ) {
    if ( storage_bitmap[i] == '1' )
      count++;
  }

  return count;
}