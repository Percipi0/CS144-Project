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

  TCPSenderMessage msg;

  if ( !syn_sent ) {
    msg.SYN = true;
    syn_sent = true;
  }

  // this->input_.read( this->reader(), cur_win_size_, msg.payload );

  if ( this->reader().is_finished() ) {
    msg.FIN = true;
    fin = true;
  }

  // return if there's no reason to send
  if ( this->reader().bytes_buffered() == 0 && !msg.SYN && !fin ) {
    return;
  }

  if ( fin && fin_received )
    return;

  msg.seqno = this->isn_ + this->reader().bytes_popped();

  if ( syn_sent && !msg.SYN )
    msg.seqno = msg.seqno + 1;

  // for ( uint64_t i = 0; i < cur_win_size_ && i < TCPConfig::MAX_PAYLOAD_SIZE; i++ ) {
  while ( true ) {
    if ( msg.sequence_length() + sequence_numbers_in_flight() >= cur_win_size_
         || this->reader().bytes_buffered() == 0 )
      break;

    // msg.payload += this->reader().peek();
    //  msg.payload = msg.payload.substr( 0, cur_win_size_ - msg.payload.length() );
    // input_.reader().pop( msg.payload.length() );
    uint64_t bytes_remaining = cur_win_size_ - ( msg.sequence_length() + sequence_numbers_in_flight() );
    string cur_str = (string)this->reader().peek();
    cur_str = cur_str.substr( 0, bytes_remaining );
    msg.payload += cur_str;
    input_.reader().pop( cur_str.length() );
  }
  //}

  if ( this->reader().is_finished() ) {
    msg.FIN = true;
    fin = true;
  }

  if ( msg.FIN && fin_queued )
    return;

  if ( msg.sequence_length() + sequence_numbers_in_flight() > cur_win_size_ && cur_win_size_ != 0 )
    return;

  if ( msg.payload == "" && !msg.SYN && !msg.FIN )
    return;

  if ( msg.FIN )
    fin_queued = true;

  transmit( msg );
  msg_queue_.push( msg );
  bytes_in_queue_ += msg.sequence_length();

  // add new outer while loop. if window size is huge, send packets of size 1000 until out of bytes or until win is
  // full
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage msg;
  msg.seqno = this->isn_ + this->reader().bytes_popped() + 1;

  if ( fin )
    msg.seqno = msg.seqno + 1;

  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // cur_win_size_ = msg.window_size < TCPConfig::MAX_PAYLOAD_SIZE ? msg.window_size : TCPConfig::MAX_PAYLOAD_SIZE;
  cur_win_size_ = msg.window_size;

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
  time_since_last_send += ms_since_last_tick;

  // if ( msg_queue_.empty() || cur_win_size_ == 0 )
  // return;

  if ( msg_queue_.empty() )
    return;

  if ( time_since_last_send >= ( initial_RTO_ms_ * pow( 2, consecutive_retransmissions() ) ) ) {
    transmit( msg_queue_.front() );
    time_since_last_send = 0;
    consec_retransmissions++;
  }
}
