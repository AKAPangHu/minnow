#include "tcp_receiver.hh"
#include "cassert"
#include "wrapping_integers.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if ( message.SYN ) {
    syn_seqno_ = message.seqno;
  } else {
    assert( syn_seqno_.has_value() );
    reassembler_.insert( message.seqno.unwrap( syn_seqno_.value(), received_byte_ ), message.payload, message.FIN );
  }

  if ( message.FIN ) {
    fin_flag_ = true;
  }

  received_byte_ += message.sequence_length();
}

TCPReceiverMessage TCPReceiver::send() const
{
  uint64_t ack_no = syn_seqno_.has_value() + reader().bytes_popped() + reader().bytes_buffered() + fin_flag_;

  return {
    TCPReceiverMessage { Wrap32::wrap( ack_no, syn_seqno_.value() ),
                         static_cast<uint16_t>(writer().available_capacity() > UINT16_MAX ? UINT16_MAX : writer().available_capacity()),
                         false } };
}
