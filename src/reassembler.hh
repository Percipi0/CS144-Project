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
  std::deque<std::tuple<uint64_t, std::string, uint64_t>> tempStorage {};
  uint64_t storageCapacity = output_.getCapacity();
  uint64_t curIndex = 0;
  bool seenLast = false;
  uint64_t finalIndex = 0;
  uint64_t bytesInStorage = 0;

  // check if byte at curIndex is already in tempStorage
  bool checkIfTempStored()
  {
    for ( uint64_t i = 0; i < tempStorage.size(); i++ ) {
      if ( get<0>( tempStorage[i] ) == curIndex )
        return true;
    }

    return false;
  }

  // returns index in tempStorage of string with desired index in bytestream, or -1 if not present
  int checkStoredAtIdx( uint64_t index )
  {
    for ( uint64_t i = 0; i < tempStorage.size(); i++ ) {
      uint64_t tempIndex = get<0>( tempStorage[i] );
      uint64_t tempEndIndex = get<2>( tempStorage[i] );
      // since tempStorage is sorted, if we reach an index above ours, we know ours isn't present
      if ( tempIndex > index )
        return -1;
      if ( tempIndex == index || ( index > tempIndex && index <= tempEndIndex ) ) {
        return i;
      }
    }

    return -1;
  }

  void putInStorage( uint64_t index, std::string data )
  {
    std::string curStr = data.substr( 0, this->output_.writer().getCapacity() - bytesInStorage );
    // uint64_t lastIndex = index + curStr.length() - 1;
    if ( tempStorage.empty() ) {
      tempStorage.push_back( { index, curStr, index + curStr.length() - 1 } );
      bytesInStorage += curStr.length();
    }

    if ( get<0>( tempStorage.back() ) < index ) {
      tempStorage.push_back( { index, curStr, index + curStr.length() - 1 } );
      bytesInStorage += curStr.length();
      return;
    }

    for ( uint64_t i = 0; i < tempStorage.size(); i++ ) {
      if ( get<0>( tempStorage[i] ) > index ) {
        tempStorage.insert( tempStorage.begin() + i, { index, curStr, index + curStr.length() - 1 } );
        bytesInStorage += curStr.length();

        /* // remove duplicates
         for ( uint64_t j = i + 1; j < tempStorage.size(); j++ ) {
           if ( get<0>( tempStorage[j] ) <= lastIndex ) {
             bytesInStorage -= get<1>( tempStorage[j] ).length();
             tempStorage.erase( tempStorage.begin() + j );
           }
         }

         for (auto elem : tempStorage) {

         }
 */
        return;
      }
    }
  }

  void removeFromStorage( uint64_t index )
  {
    for ( uint64_t i = 0; i < tempStorage.size(); i++ ) {
      if ( get<0>( tempStorage[i] ) == index ) {
        bytesInStorage -= get<1>( tempStorage[i] ).length();
        tempStorage.erase( tempStorage.begin() + i );
        break;
      }
    }
  }

  // remove unneeded elements from the beginning of tempStorage
  void cleanStorage()
  {
    while ( !tempStorage.empty() && get<0>( tempStorage.front() ) < curIndex ) {
      bytesInStorage -= ( get<1>( tempStorage.front() ).length() );
      tempStorage.pop_front();
    }
  }

  void clearStorageOverlap( uint64_t firstIndex, uint64_t length )
  {
    for ( uint64_t i = firstIndex; i < length; i++ ) {
      int curIdx = checkStoredAtIdx( i );
      if ( curIdx != -1 ) {
        removeFromStorage( i );
      }
    }
  }

  void clearStorageOverlap2( uint64_t firstIndex, uint64_t lastIndex )
  {
    for ( uint64_t i = firstIndex; i <= lastIndex; i++ ) {
      removeFromStorage( i );
    }
  }

  uint64_t lastIndex() { return ( ( curIndex + storageCapacity ) - 1 ); }

  bool isValidIndex( uint64_t index )
  {
    return ( ( index >= curIndex ) && ( index < curIndex + storageCapacity ) );
  }
};
