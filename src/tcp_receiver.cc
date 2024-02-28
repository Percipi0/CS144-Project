#include "tcp_receiver.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"
#include <cstdint>
#include <iostream>
#include <iterator>
#include <optional>
#include <type_traits>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{

  if ( message.RST )
    this->reader().set_error();

  if ( !message.SYN && !seen_syn )
    return;

  if ( message.SYN ) {
    isn_ = message.seqno;
    cur_seq = isn_;
    seen_syn = true;
  }

  if ( message.FIN ) {
    seen_fin = true;
  }

  bool is_last_substring = message.FIN ? true : false;

  prev_bytes_pending = this->reassembler_.bytes_pending();
  prev_bytes_pushed = this->writer().bytes_pushed();

  uint64_t message_seqno_unwrapped = message.seqno.unwrap( isn_, this->writer().bytes_pushed() );
  uint64_t cur_seq_unwrapped = cur_seq.unwrap( isn_, this->writer().bytes_pushed() );

  if ( seen_syn && !message.payload.empty() ) {
    // message is further ahead in sequence, so we need to send it over but not update cur_seq
    if ( message_seqno_unwrapped > cur_seq_unwrapped ) {
      // subtract 1 from first_index to account for SYN offset
      reassembler_.insert(
        message.seqno.unwrap( isn_, this->writer().bytes_pushed() ) - 1, message.payload, is_last_substring );
      // bytes_not_in_seq += ( reassembler_.bytes_pending() - prev_bytes_pending );

      return;
    }

    if ( message_seqno_unwrapped < cur_seq_unwrapped ) {
      // current message is behind cur_seq
      reassembler_.insert( message_seqno_unwrapped - 1, message.payload, is_last_substring );
    } else {
      if ( cur_seq == isn_ ) {
        // haven't incremented cur_seq at all, so we'll need to avoid underflow error
        reassembler_.insert( cur_seq_unwrapped, message.payload, is_last_substring );
      } else {
        // normal insert, still subtract 1 to account for SYN
        reassembler_.insert( cur_seq_unwrapped - 1, message.payload, is_last_substring );
      }
    }
  } else {
    // empty payload, SYN already accounted for in cur_seq
    if ( cur_seq != isn_ ) {
      reassembler_.insert( cur_seq_unwrapped - 1, message.payload, is_last_substring );
    } else {

      // empty payload, SYN not yet accounted for
      reassembler_.insert( cur_seq_unwrapped, message.payload, is_last_substring );
    }
  }
  cur_seq = cur_seq + ( this->writer().bytes_pushed() - prev_bytes_pushed );

  if ( seen_syn && !syn_accounted ) {
    cur_seq = cur_seq + 1;
    syn_accounted = true;
  }

  if ( seen_fin && !fin_accounted ) {
    cur_seq = cur_seq + 1;
    fin_accounted = true;
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
  uint32_t real_capacity = capacity > UINT16_MAX ? UINT16_MAX : capacity;
  real_capacity -= this->reader().bytes_buffered();

  bool err = false;
  if ( this->reassembler_.reader().has_error() )
    err = true;

  if ( !seen_syn )
    return TCPReceiverMessage( empty_wrap, real_capacity, err );

  return TCPReceiverMessage( cur_seq, real_capacity, err );
}
