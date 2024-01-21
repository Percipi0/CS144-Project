#include "reassembler.hh"
#include <cstdint>
#include <iostream>
#include <sys/types.h>

using namespace std;

// maybe use string as tempStorage and use bitmap to keep track of which bytes in string are actually occupied?

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  /*
  cerr << "---" << endl;
  cerr << "curIndex: " << curIndex << endl;
  cerr << "first index:" << first_index << endl;
  cerr << "first index in tempStorage: " << get<0>( tempStorage.front() ) << endl;
  cerr << "current data length:" << data.length() << endl;
  cerr << "bytes in tempStorage:" << bytes_pending() << endl;
  cerr << "---" << endl;
*/
  if ( this->output_.writer().is_closed() )
    return;

  // if we have curindex, push immediately
  if ( first_index == curIndex ) {
    // push as much data as we can
    string tempStr = data.substr( 0, this->output_.writer().available_capacity() );
    this->output_.writer().push( tempStr );
    curIndex += tempStr.length();

  } else if ( first_index > curIndex ) {
    // check if we've put an elem at the edge of the buffer, and whether we're trying to go beyond that
    if ( !isValidIndex( first_index ) ) {
      return;
    };
    int curIdx = checkStoredAtIdx( first_index );
    if ( curIdx != -1 ) {
      // there must be overlap within the string stored in the tuple at curIdx in tempStorage
      if ( first_index > (uint64_t)curIdx ) {
        // iterate thru data and attempt to find/add non overlap portions
        for ( uint64_t i = 1; i < data.length(); i++ ) {
          int overallIndex = -1;
          int strStart = -1;
          uint64_t numNewBytes = 0;

          int tempIdx = checkStoredAtIdx( first_index + i );
          if ( tempIdx == -1 ) {
            overallIndex = first_index + i;
            strStart = i;
            numNewBytes++;
            for ( uint64_t j = i + 1; j < data.length(); j++ ) {
              int tempEndIdx = checkStoredAtIdx( first_index + j );
              if ( tempEndIdx == -1 ) {
                numNewBytes++;
              } else
                break;
            }
          }
          if ( strStart != -1 ) {
            string curStr = data.substr( strStart, numNewBytes );
            string realStr = curStr.substr( 0, output_.writer().getCapacity() - bytesInStorage );
            putInStorage( overallIndex, realStr );
          }
        }
        // replace data at occupied index with data of greater length at same index
      } else if ( ( get<1>( tempStorage[curIdx] ).length() ) < data.length() ) {
        removeFromStorage( first_index );
        putInStorage( first_index, data );
        clearStorageOverlap( first_index + 1, first_index + data.length() - 1 );
      }

    } else {
      // index not yet in storage
      putInStorage( first_index, data.substr( 0, lastIndex() - first_index + 1 ) );
      clearStorageOverlap( first_index + 1, first_index + data.length() - 1 );
    }
  } else {
    // first_index must be less than curIndex
    for ( uint64_t i = 0; i < data.length(); i++ ) {
      if ( ( first_index + i ) == curIndex ) {
        // push as much as possible!
        string tempStr = data.substr( i, this->output_.writer().available_capacity() );
        this->output_.writer().push( tempStr );
        curIndex += tempStr.length();
        break;
      }
    }
  }

  // see if it's time to push tempStorage contents
  // sometimes we never add new bytes bc they start out with bytes already pushed
  if ( !this->tempStorage.empty() ) {
    for ( uint64_t i = 0; i < this->tempStorage.size(); i++ ) {
      if ( get<0>( tempStorage[i] ) == curIndex ) {
        string tempStr = get<1>( tempStorage[i] ).substr( 0, this->output_.writer().available_capacity() );
        this->output_.writer().push( tempStr );
        curIndex += tempStr.length();
      } else if ( get<0>( tempStorage[i] ) < curIndex ) {
        auto curElem = tempStorage[i];
        if ( curIndex <= get<2>( curElem ) ) {
          for ( uint64_t j = 0; j < get<1>( curElem ).length(); j++ ) {
            if ( get<0>( curElem ) + j == curIndex ) {
              string tempStr = get<1>( tempStorage[i] ).substr( j, this->output_.writer().available_capacity() );
              this->output_.writer().push( tempStr );
              curIndex += tempStr.length();
              break;
            }
          }
        }
      } else
        break;
    }
  }

  while ( true ) {
    if ( tempStorage.empty() )
      break;
    if ( get<0>( tempStorage.front() ) < curIndex ) {
      bytesInStorage -= get<1>( tempStorage.front() ).length();
      tempStorage.pop_front();
    } else
      break;
  }

  if ( is_last_substring ) {
    seenLast = true;
    if ( data.length() != 0 ) {
      finalIndex = first_index + data.length() - 1;
    }
  }

  if ( seenLast && tempStorage.size() == 0 && ( curIndex > finalIndex || finalIndex == 0 ) ) {
    this->output_.writer().close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return bytesInStorage;
}