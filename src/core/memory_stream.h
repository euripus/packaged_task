#ifndef MEMORYSTREAM_H
#define MEMORYSTREAM_H

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace evnt
{
class OutputMemoryStream
{
public:
    const int8_t * getBufferPtr() const { return _buffer.data(); }
    uint32_t       getLength() const { return _buffer.size(); }

    void write(const int8_t * inData, size_t inByteCount);

    template<typename T>
    void write(T inData)
    {
        static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>,
                      "Generic Write only supports primitive data types");

        write(reinterpret_cast<const int8_t *>(&inData), sizeof(inData));
    }

    void write(const std::string & inString)
    {
        uint32_t elementCount = inString.size();
        write(elementCount);
        write(reinterpret_cast<const int8_t *>(inString.data()), elementCount * sizeof(char));
    }

private:
    std::vector<int8_t> _buffer;
};

class InputMemoryStream
{
public:
    InputMemoryStream(std::unique_ptr<int8_t[]> inData, size_t inByteCount) :
        _data{std::move(inData)},
        _head{0},
        _capacity{inByteCount}
    {}
    InputMemoryStream(size_t inByteCount) :
        _data{std::make_unique<int8_t[]>(inByteCount)},
        _head{0},
        _capacity{inByteCount}
    {}
    InputMemoryStream()  = default;
    ~InputMemoryStream() = default;

    InputMemoryStream(const InputMemoryStream & other) : _head{other._head}, _capacity{other._capacity}
    {
        _data = std::make_unique<int8_t[]>(_capacity);
        std::memcpy(_data.get(), other._data.get(), _capacity);
    }
    InputMemoryStream & operator=(const InputMemoryStream & other)
    {
        _head     = other._head;
        _capacity = other._capacity;
        _data     = std::make_unique<int8_t[]>(_capacity);
        std::memcpy(_data.get(), other._data.get(), _capacity);

        return *this;
    }

    InputMemoryStream(InputMemoryStream && other) : InputMemoryStream() { std::swap(*this, other); }
    InputMemoryStream & operator=(InputMemoryStream && other)
    {
        std::swap(*this, other);

        return *this;
    }

    friend void swap(InputMemoryStream & left, InputMemoryStream & right)
    {
        using std::swap;

        swap(left._data, right._data);
        swap(left._head, right._head);
        swap(left._capacity, right._capacity);
    }

    int32_t getRemainingDataSize() const { return _capacity - _head; }

    void read(void * outData, uint32_t inByteCount) const;

    template<typename T>
    void read(T & outData) const
    {
        static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>,
                      "Generic Read only supports primitive data types");
        read(reinterpret_cast<void *>(&outData), sizeof(outData));
    }

    void read(std::string & inString) const
    {
        inString.clear();
        uint32_t elementCount;
        read(&elementCount, sizeof(uint32_t));
        inString.resize(elementCount);
        read(inString.data(), elementCount * sizeof(char));
    }

    int8_t * getCurPosPtr() const;
    void     resetHead() { _head = 0; }
    void     setCapacity(uint32_t newCapacity) { _capacity = newCapacity; }

private:
    std::unique_ptr<int8_t[]> _data;
    mutable size_t            _head;
    size_t                    _capacity;
};
}   // namespace evnt

#endif   // MEMORYSTREAM_H
