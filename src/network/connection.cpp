#include "connection.h"
#include "../log/log.h"

namespace evnt
{
extern int64_t                 GetMilisecFromStart();
static boost::asio::io_service io_serv;

Connection::Connection() : mSocket{std::make_shared<UDPSocket>(io_serv)} {}

void Connection::processIncomingPackets()
{
    readIncomingPacketsIntoQueue();
    processQueuedPackets();
}

void Connection::readIncomingPacketsIntoQueue()
{
    // should we just keep a static one?
    static const int         packetSize = 1500;
    static InputMemoryStream inputStream(packetSize);
    SocketAddress            fromAddress;

    // keep reading until we don't have anything to read
    // (or we hit a max number that we'll process per frame)
    int receivedPackedCount = 0;
    int totalReadByteCount  = 0;

    while(receivedPackedCount < kMaxPacketsPerFrameCount)
    {
        try
        {
            size_t readByteCount = mSocket->receiveFrom(inputStream.getCurPosPtr(), packetSize, fromAddress);
            if(readByteCount > 0)
            {
                inputStream.setCapacity(readByteCount);
                ++receivedPackedCount;
                totalReadByteCount += readByteCount;

                uint32_t simulatedReceivedTime = GetMilisecFromStart();
                mPacketQueue.emplace(simulatedReceivedTime, inputStream, fromAddress);
                inputStream.resetHead();
            }
            else
                break;
        }
        catch(boost::system::system_error & err)
        {
            if(err.code() == boost::asio::error::connection_reset)
            {
                // port closed on other end, so DC this person immediately
                handleConnectionReset(fromAddress);
            }

            std::stringstream ss;
            ss << "Error reading from address: " << fromAddress.toString() << ". " << err.what() << std::endl;
            Log::Log(Log::error, ss.str());
        }
    }
}

void Connection::processQueuedPackets()
{
    // look at the front packet...
    while(!mPacketQueue.empty())
    {
        ReceivedPacket & nextPacket = mPacketQueue.front();
        if(GetMilisecFromStart() > nextPacket.getReceivedTime())
        {
            processPacket(nextPacket.getPacketBuffer(), nextPacket.getFromAddress());
            mPacketQueue.pop();
        }
        else
        {
            break;
        }
    }
}

void Connection::sendPacket(const OutputMemoryStream & inOutputStream, const SocketAddress & inToAddress)
{
    int sentByteCount =
        mSocket->sendTo(inOutputStream.getBufferPtr(), inOutputStream.getLength(), inToAddress);
    if(sentByteCount > 0)
    {
        // mBytesSentThisFrame += sentByteCount;
    }
}
}   // namespace evnt
