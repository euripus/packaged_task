#include "udpsocket.h"
#include "../log/log.h"

namespace evnt
{
bool UDPSocket::bind(const SocketAddress & inToAddress)
{
    bool res = false;
    try
    {
        m_socket.bind(inToAddress.m_endpoint);
        res = true;
    }
    catch(boost::system::system_error & err)
    {
        std::stringstream ss;
        ss << "Error binding to address: " << inToAddress.toString() << ". " << err.what() << std::endl;
        Log::Log(Log::error, ss.str());
    }

    return res;
}

size_t UDPSocket::sendTo(const void * inToSend, size_t inLength, const SocketAddress & inToAddress)
{
    return m_socket.send_to(boost::asio::buffer(inToSend, inLength), inToAddress.m_endpoint);
}

size_t UDPSocket::receiveFrom(void * inToReceive, size_t inMaxLength, SocketAddress & outFromAddress)
{
    return m_socket.receive_from(boost::asio::buffer(inToReceive, inMaxLength), outFromAddress.m_endpoint);
}
}   // namespace evnt
