#pragma once

#include "reassembler.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"
#include <cstdint>
#include <sys/types.h>

class TCPReceiver
{
public:
  // Construct with given Reassembler
  explicit TCPReceiver( Reassembler&& reassembler ) : reassembler_( std::move( reassembler ) ) {}

  /*
   * The TCPReceiver receives TCPSenderMessages, inserting their payload into the Reassembler
   * at the correct stream index.
   */
  void receive( TCPSenderMessage message );

  // The TCPReceiver sends TCPReceiverMessages to the peer's TCPSender.
  TCPReceiverMessage send() const;

  // Access the output (only Reader is accessible non-const)
  const Reassembler& reassembler() const { return reassembler_; }
  Reader& reader() { return reassembler_.reader(); }
  const Reader& reader() const { return reassembler_.reader(); }
  const Writer& writer() const { return reassembler_.writer(); }

private:
  Reassembler reassembler_;
  Wrap32 isn_ = Wrap32( 0 );
  std::optional<Wrap32> empty_wrap = {};
  Wrap32 cur_seq = Wrap32( 0 );
  uint32_t capacity = reassembler_.writer().available_capacity();
  bool seen_syn = false;
  bool syn_accounted = false;
  bool seen_fin = false;
  bool fin_accounted = false;
  uint64_t bytes_not_in_seq = 0;
  uint64_t prev_bytes_pending = 0;
  uint64_t prev_bytes_pushed = 0;
};
