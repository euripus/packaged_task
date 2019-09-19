#include "exception.h"
#include <sstream>

namespace evnt
{
const char * Exception::what() const noexcept
{
    std::ostringstream buffer;
    buffer << "File: \"" << eFile_ << "\"  Line: " << eLine_ << "\n"
           << "\tMessage: \"" << eText_ << "\"\n";
    buffer.flush();
    message_ = buffer.str();

    return message_.c_str();
}
}   // namespace evnt
