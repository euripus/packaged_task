#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "file.h"
#include <functional>
#include <list>
#include <vector>

namespace evnt
{
class FileSystem
{
public:
    using FilePtr       = std::unique_ptr<IFile>;
    using MemoryFilePtr = std::unique_ptr<MemoryFile>;

    FileSystem(std::string root_dir);
    virtual ~FileSystem() = default;

    bool    isExist(const std::string & fname) const;
    FilePtr getFile(const std::string & fname) const;   // ex. file name: "fonts/times.ttf"
    size_t  getNumFiles() const { return m_files.size(); }

    bool writeFile(const std::string & path, FilePtr file);   // Memory file
    bool createZIP(std::vector<FilePtr> filelist,
                   const std::string &  zipname);   // all zip files saves in root directory
    bool addFileToZIP(FilePtr file, const std::string & zipname);

    static std::string GetTempDir();
    static std::string GetCurrentDir();
    static std::string GetTempFileName();

private:
    struct file_data
    {
        struct ZFileData
        {
            bool        compressed       = false;
            size_t      compressedSize   = 0;
            size_t      uncompressedSize = 0;
            size_t      lfhOffset        = 0;
            std::string fname;
        };

        bool        is_zip = false;
        ZFileData   zip_data;
        std::string fname;
    };

    void    addZippedDir(const std::string & fname);
    FilePtr loadPermanentFile(const file_data & f) const;
    FilePtr loadZipFile(const file_data & zf) const;

    std::list<file_data> m_files;
    std::string          m_data_dir;
};
}   // namespace evnt

#endif   // FILESYSTEM_H
