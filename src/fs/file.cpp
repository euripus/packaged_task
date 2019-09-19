#include "file.h"
#include "../log/log.h"
#include "file_system.h"
#include <cassert>
#include <chrono>
#include <cstring>
#include <fstream>
#include <zlib.h>

//#include "zip.h"

namespace evnt
{
MemoryFile::MemoryFile()
{
    m_name           = FileSystem::GetTempFileName();
    m_last_write_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

MemoryFile::MemoryFile(std::string name)
{
    std::swap(m_name, name);
    m_last_write_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

MemoryFile::MemoryFile(std::string name, const char * data, size_t length)
{
    std::swap(m_name, name);
    m_data.reserve(length);
    std::copy(data, data + length, std::back_inserter(m_data));
    m_last_write_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

void MemoryFile::write(const char * buffer, size_t len)
{
    assert(buffer != nullptr);
    assert(len > 0);

    m_data.reserve(len);
    std::copy(buffer, buffer + len, std::back_inserter(m_data));
    m_last_write_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}
}   // namespace evnt
