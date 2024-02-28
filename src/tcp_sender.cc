#include "tcp_sender.hh"
#include "byte_stream.hh"
#include "iostream"
#include "tcp_config.hh"
#include "tcp_sender_message.hh"
#include <cstdint>
#include <math.h>
#include <sys/types.h>

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return bytes_in_queue_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return consec_retransmissions;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  while ( true ) {

    TCPSenderMessage msg;

    if ( this->reader().has_error() ) {
      msg.RST = true;
    }

    if ( !syn_sent ) {
      msg.SYN = true;
      syn_sent = true;
    }

    // set FIN flag if reader is finished AND we have space for FIN.
    // This conditional is required in the case that we need to send FIN but don't have bytes buffered
    if ( this->reader().is_finished()
         && ( ( msg.sequence_length() + sequence_numbers_in_flight() < cur_win_size_ ) || cur_win_size_ == 0 ) ) {
      msg.FIN = true;
      fin = true;
    }

    // return if there's no good reason to send OR if we've already received an ack for FIN
    if ( ( this->reader().bytes_buffered() == 0 && !msg.SYN && !fin ) || ( fin && fin_received ) ) {
      return;
    }

    msg.seqno = this->isn_ + this->reader().bytes_popped();

    if ( syn_sent && !msg.SYN )
      msg.seqno = msg.seqno + 1;

    while ( true ) {

      // stop adding to the current message if we're out of space or data to add
      if ( msg.sequence_length() + sequence_numbers_in_flight() >= cur_win_size_
           || this->reader().bytes_buffered() == 0 || msg.payload.size() == TCPConfig::MAX_PAYLOAD_SIZE ) {
        if ( cur_win_size_ != 0 )
          break;
      }

      if ( cur_win_size_ == 0 && msg.sequence_length() == 1 )
        break;

      uint64_t bytes_remaining;
      if ( cur_win_size_ == 0 ) {
        // treat window size as 1 instead of 0, only if queue is empty
        if ( msg_queue_.empty() ) {
          bytes_remaining = 1;
        } else
          break;
      } else {
        bytes_remaining = cur_win_size_ - ( msg.sequence_length() + sequence_numbers_in_flight() );

        if ( bytes_remaining > TCPConfig::MAX_PAYLOAD_SIZE ) {
          bytes_remaining = TCPConfig::MAX_PAYLOAD_SIZE;
        }
      }

      string cur_str = (string)this->reader().peek();
      cur_str = cur_str.substr( 0, bytes_remaining );
      msg.payload += cur_str;
      input_.reader().pop( cur_str.length() );
    }

    // add FIN if the requirements are met AND we have the space
    if ( this->reader().is_finished() && msg.sequence_length() + sequence_numbers_in_flight() < cur_win_size_ ) {
      msg.FIN = true;
      fin = true;
    }

    // avoid queueing two FINs simultaneously
    if ( msg.FIN && fin_queued )
      return;

    /* if ( msg.payload == "" && !msg.SYN && !msg.FIN ) {
       if ( cur_win_size_ == 0 ) {
         transmit( msg );
         msg_queue_.push( msg );
         bytes_in_queue_ += msg.sequence_length();
         return;
       } else
         return;
     }*/

    // avoid sending message when window size cannot accomodate
    if ( msg.sequence_length() + sequence_numbers_in_flight() > cur_win_size_ && cur_win_size_ != 0 )
      return;

    if ( msg.payload == "" && !msg.SYN && !fin )
      return;

    // mark FIN as having been queued to avoid queueing more than one FIN
    if ( msg.FIN )
      fin_queued = true;

    transmit( msg );
    msg_queue_.push( msg );
    bytes_in_queue_ += msg.sequence_length();

    // break out of loop if we're out of either bytes buffered or space in the window
    if ( this->reader().bytes_buffered() == 0 || sequence_numbers_in_flight() == cur_win_size_ )
      return;
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage msg;
  msg.seqno = this->isn_ + this->reader().bytes_popped() + 1;

  if ( fin )
    msg.seqno = msg.seqno + 1;

  if ( this->reader().has_error() ) {
    msg.RST = true;
  }

  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  cur_win_size_ = msg.window_size;

  if ( msg.window_size == 0 ) {
    win_advertised_zero = true;
  } else {
    win_advertised_zero = false;
  }

  if ( msg.RST ) {
    this->writer().set_error();
  }

  while ( true ) {
    if ( msg_queue_.empty() )
      break;

    auto cur_msg = msg_queue_.front();
    auto cur_seqno = cur_msg.seqno.unwrap( this->isn_, this->input_.writer().bytes_pushed() );
    auto next_seqno = msg.ackno->unwrap( this->isn_, this->input_.writer().bytes_pushed() );

    if ( cur_seqno + cur_msg.sequence_length() == next_seqno
         || cur_seqno + sequence_numbers_in_flight() == next_seqno ) {

      if ( cur_msg.FIN ) {
        fin_received = true;
      }

      bytes_in_queue_ -= msg_queue_.front().sequence_length();
      consec_retransmissions = 0;
      time_since_last_send = 0;
      msg_queue_.pop();
    } else
      break;
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{

  if ( msg_queue_.empty() )
    return;

  time_since_last_send += ms_since_last_tick;

  if ( time_since_last_send >= ( initial_RTO_ms_ * pow( 2, consecutive_retransmissions() ) ) ) {

    time_since_last_send = 0;

    transmit( msg_queue_.front() );

    // why is this not incremented by 1?
    if ( !win_advertised_zero )
      consec_retransmissions++;

    // break 418, check here.. we return too early and dont increment consec
    //  if ( cur_win_size_ - sequence_numbers_in_flight() == 0 )
    //  return;
  }
}
