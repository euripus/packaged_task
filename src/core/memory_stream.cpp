#include "memory_stream.h"

#include <cstring>
#include <stdexcept>

namespace evnt
{
void OutputMemoryStream::write(const int8_t * inData, size_t inByteCount)
{
    _buffer.reserve(_buffer.size() + inByteCount);
    _buffer.insert(std::end(_buffer), inData, inData + inByteCount);
}

void InputMemoryStream::read(void * outData, uint32_t inByteCount) const
{
    uint32_t resultHead = _head + inByteCount;
    if(resultHead > _capacity)
    {
        throw std::range_error("InputMemoryStream::Read - no data to read!");
    }

    std::memcpy(outData, _data.get() + _head, inByteCount);

    _head = resultHead;
}

int8_t * InputMemoryStream::getCurPosPtr() const
{
    return  _data.get() + _head;
}
}   // namespace evnt
