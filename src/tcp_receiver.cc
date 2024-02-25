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

  // 丢弃不合法的报文，也就是说要保证窗口外的数据是不会被接收的
  uint64_t seqno_u64_t = message.seqno.unwrap( syn_seqno_.value(), calculate_ackno() );

  if ( !message.SYN ) {
    if ( seqno_u64_t < calculate_ackno() ) {
      return;
    }

    if ( seqno_u64_t + 1 > calculate_ackno() + calculate_window_size() ) {
      return;
    }
  }

  if ( message.FIN ) {
    fin_flag_ = true;
  }

  reassembler_.insert( seqno_u64_t == 0 ? 0 : seqno_u64_t - 1, message.payload, message.FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{

  uint64_t ack_no = calculate_ackno();

  return { TCPReceiverMessage {
    syn_seqno_.has_value() ? make_optional<>( Wrap32::wrap( ack_no, syn_seqno_.value() ) ) : nullopt,
    static_cast<uint16_t>( calculate_window_size() ),
    rst_flag_ } };
}
uint64_t TCPReceiver::calculate_window_size() const
{
  return writer().available_capacity() > UINT16_MAX ? UINT16_MAX : writer().available_capacity();
}

uint64_t TCPReceiver::calculate_ackno() const
{
  uint64_t ack_no = syn_seqno_.has_value() + reader().bytes_popped() + reader().bytes_buffered() + fin_flag_;
  return ack_no;
}
