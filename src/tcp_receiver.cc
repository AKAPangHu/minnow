#include "tcp_receiver.hh"
#include "cassert"
#include "wrapping_integers.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{

  if ( message.seqno.unwrap(syn_seqno_.value(), received_byte_ ) < last_ackno_ ){
    return;
  }

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
  reassembler_.insert( now_u64_t == 0 ? 0 : now_u64_t - 1, message.payload, message.FIN );
  received_byte_ += message.sequence_length();
}

TCPReceiverMessage TCPReceiver::send()
{

  uint64_t ack_no = syn_seqno_.has_value() + reader().bytes_popped() + reader().bytes_buffered() + fin_flag_;

  if ( ack_no > last_ackno_ ) {
    last_ackno_ = ack_no;
  }

  return { TCPReceiverMessage {
    syn_seqno_.has_value() ? make_optional<>( Wrap32::wrap( ack_no, syn_seqno_.value() ) ) : nullopt,
    static_cast<uint16_t>( writer().available_capacity() > UINT16_MAX ? UINT16_MAX
                                                                      : writer().available_capacity() ),
    rst_flag_ } };
}
