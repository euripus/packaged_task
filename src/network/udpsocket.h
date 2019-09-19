#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include "socketaddress.h"
#include <boost/asio.hpp>
#include <memory>

namespace evnt
{
class UDPSocket
{
    boost::asio::ip::udp::socket m_socket;

public:
    UDPSocket(boost::asio::io_service & io) : m_socket(io) {}
    ~UDPSocket() = default;

    bool   bind(const SocketAddress & inToAddress);
    size_t sendTo(const void * inToSend, size_t inLength, const SocketAddress & inToAddress);
    size_t receiveFrom(void * inToReceive, size_t inMaxLength, SocketAddress & outFromAddress);
};

using UDPSocketPtr = std::shared_ptr<UDPSocket>;
}   // namespace evnt
#endif   // UDPSOCKET_H
