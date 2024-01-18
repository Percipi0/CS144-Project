#include "reassembler.hh"
#include <iostream>
#include <algorithm>

using namespace std;
/*
    std::tuple<uint64_t, std::string> elem = std::ranges::find_if(this->tempStorage.begin(), this->tempStorage.end(), [this] (std::tuple<uint64_t, std::string> curElem) { return std::get<0>(curElem) == curIndex; } );
    cout << elem << endl;

              //write to the reassembler's storage for now


                  if (this->storageCapacity - this->tempStorage.size() == 0) return;
        
        this->tempStorage.push_back({(uint64_t)(first_index+i), data.substr(i,1)});
        sort(tempStorage.begin(), tempStorage.end());

  if ((first_index < curIndex) && data.size() <= curIndex) {
    return;
    } else {
      first_index = curIndex;
    }

*/

//issue: we push a, curindex is set to higher than first index while str is still in storage. so we never clear the
//above if statement, meaning we never empty tempstorage

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if (this->output_.writer().is_closed()) return;

    if (!this->tempStorage.empty()) {
    for (uint64_t i = curIndex; i < this->tempStorage.size(); i++) {
      //if (this->output_.writer().available_capacity() == 0) break;
      if (tempStorage[i] != NULL) {
        break;
      } else {
        this->output_.writer().push(tempStorage[i]);
        curIndex++;
      }
    }
  }

  if (first_index == curIndex) {
    if (this->output_.writer().available_capacity() != 0) {
    for (uint64_t i = 0; i < data.length(); i++) {
      //bytestream buffer filled up during the processing of string
      if (this->output_.writer().available_capacity() == 0) {
        return;
      } else {
        this->output_.writer().push(data.substr(i, 1));
        curIndex++;
    }
    }
  }
  } else if (first_index > curIndex) {
    for (uint64_t i = 0; i < data.length(); i++) {
      if (this->output_.writer().available_capacity() - this->tempStorage.size() == 0) return;
      if ((first_index+i) > this->tempStorage.size()-1) return; 
      this->tempStorage[first_index+i] = data.substr(i,1);
      }
      //sort(tempStorage.begin(), tempStorage.end());
  } else {
    //first_index must be less than curIndex
    if (data.size() > curIndex) {
          for (uint64_t i = data.size()-curIndex-1; i < data.length(); i++) {
      //bytestream buffer filled up during the processing of string
      if (this->output_.writer().available_capacity() == 0) {
        return;
      } else {
        this->output_.writer().push(data.substr(i, 1));
        curIndex++;
    }
    }
    }
  }

  if (is_last_substring) {
    this->output_.writer().close();
    }
}

uint64_t Reassembler::bytes_pending() const
{
  return tempStorage.size();
}


