#ifndef FILE_H
#define FILE_H

#include <ctime>
#include <memory>
#include <string>
#include <vector>

namespace evnt
{
class IFile
{
public:
    virtual ~IFile() = default;

    virtual const int8_t * getData() const     = 0;
    virtual size_t         getFileSize() const = 0;
    virtual std::time_t    timeStamp() { return m_last_write_time; }
    virtual std::string    getName() const = 0;

protected:
    std::time_t m_last_write_time = 0;
};

class MemoryFile : public IFile
{
public:
    MemoryFile();   // file for writing with random name
    MemoryFile(std::string name);
    MemoryFile(std::string name, const char * data, size_t length);
    ~MemoryFile() override = default;

    const int8_t * getData() const override { return m_data.data(); }
    size_t         getFileSize() const override { return m_data.size(); }
    std::string    getName() const override { return m_name; }

    void write(const char * buffer, size_t len);

private:
    std::string         m_name;
    std::vector<int8_t> m_data;
};

class BufferedFile : public IFile
{
public:
    BufferedFile(std::string name, std::time_t timestamp, size_t f_size, std::unique_ptr<int8_t[]> data) :
        m_file_size(f_size)
    {
        std::swap(m_name, name);
        m_last_write_time = timestamp;
        std::swap(m_data, data);
    }
    ~BufferedFile() override = default;

    const int8_t * getData() const override { return m_data.get(); }
    size_t         getFileSize() const override { return m_file_size; }
    std::string    getName() const override { return m_name; }

private:
    std::string               m_name;
    size_t                    m_file_size = 0;
    std::unique_ptr<int8_t[]> m_data;
};
}   // namespace evnt
#endif   // FILE_H
