#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "../core/memory_stream.h"
#include "udpsocket.h"
#include <list>
#include <queue>

namespace evnt
{
class Connection
{
public:
    static const int kMaxPacketsPerFrameCount = 10;

    Connection();
    virtual ~Connection() = default;

    bool init(uint16_t inPort);
    void processIncomingPackets();

    virtual void processPacket(InputMemoryStream & inInputStream, const SocketAddress & inFromAddress) = 0;
    virtual void handleConnectionReset(const SocketAddress & inFromAddress) { (void)inFromAddress; }

    void sendPacket(const OutputMemoryStream & inOutputStream, const SocketAddress & inToAddress);

private:
    // void	UpdateBytesSentLastFrame();
    void readIncomingPacketsIntoQueue();
    void processQueuedPackets();

    class ReceivedPacket
    {
    public:
        ReceivedPacket(uint32_t inReceivedTime, InputMemoryStream & inInputMemoryStream,
                       const SocketAddress & inAddress) :
            mReceivedTime{inReceivedTime}, mPacketBuffer{inInputMemoryStream}, mFromAddress{inAddress}
        {}

        const SocketAddress & getFromAddress() const { return mFromAddress; }
        uint32_t              getReceivedTime() const { return mReceivedTime; }
        InputMemoryStream &   getPacketBuffer() { return mPacketBuffer; }

    private:
        uint32_t          mReceivedTime;
        InputMemoryStream mPacketBuffer;
        SocketAddress     mFromAddress;
    };

    std::queue<ReceivedPacket, std::list<ReceivedPacket>> mPacketQueue;
    UDPSocketPtr                                          mSocket;
};
}   // namespace evnt

#endif   // NETWORKMANAGER_H
