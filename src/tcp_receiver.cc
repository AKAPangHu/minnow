#include "tcp_receiver.hh"
#include "cassert"
#include "wrapping_integers.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{

  if ( message.SYN ) {
    syn_seqno_ = message.seqno;
  }

  if ( !syn_seqno_.has_value() ) {
    rst_flag_ = true;
    return;
  };

  if ( message.FIN ) {
    fin_flag_ = true;
  }

  uint64_t now_u64_t = message.seqno.unwrap( syn_seqno_.value(), received_byte_ );
  reassembler_.insert( now_u64_t, message.payload, message.FIN );
  received_byte_ += message.sequence_length();
}

TCPReceiverMessage TCPReceiver::send() const
{

  uint64_t ack_no = syn_seqno_.has_value() + reader().bytes_popped() + reader().bytes_buffered() + fin_flag_;

  return { TCPReceiverMessage {
    syn_seqno_.has_value() ? make_optional<>( Wrap32::wrap( ack_no, syn_seqno_.value() ) ) : nullopt,
    static_cast<uint16_t>( writer().available_capacity() > UINT16_MAX ? UINT16_MAX
                                                                      : writer().available_capacity() ),
    rst_flag_ } };
}
