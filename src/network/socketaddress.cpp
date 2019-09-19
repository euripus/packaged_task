#include "socketaddress.h"
#include <sstream>

namespace evnt
{
std::string SocketAddress::toString() const
{
    std::stringstream ss;
    ss << getAddress() << ":" << getPort();
    return ss.str();
}
}   // namespace evnt
