#ifndef ZIP_H_INCLUDED
#define ZIP_H_INCLUDED

#include <cstdint>

#pragma pack(push, 1)
struct EOCD   // End of central directory record
{
    uint16_t diskNumber;
    uint16_t startDiskNumber;
    uint16_t numberCentralDirectoryRecord;
    uint16_t totalCentralDirectoryRecord;
    uint32_t sizeOfCentralDirectory;
    uint32_t centralDirectoryOffset;
    uint16_t commentLength;
    // zip file comment follows
};

struct CentralDirectoryFileHeader
{
    uint32_t signature;                // PK12
    uint16_t versionMadeBy;            // version made by
    uint16_t versionToExtract;         // version needed to extract
    uint16_t generalPurposeBitFlag;    // general purpose bit flag
    uint16_t compressionMethod;        // compression method
    uint16_t modificationTime;         // last mod file time
    uint16_t modificationDate;         // last mod file date
    uint32_t crc32;                    // crc-32
    uint32_t compressedSize;           // compressed size
    uint32_t uncompressedSize;         // uncompressed size
    uint16_t filenameLength;           // file name length
    uint16_t extraFieldLength;         // extra field length
    uint16_t fileCommentLength;        // file comment length
    uint16_t diskNumber;               // disk number start
    uint16_t internalFileAttributes;   // internal file attributes
    uint32_t externalFileAttributes;   // external file attributes
    uint32_t localFileHeaderOffset;    // relative offset of local header
                                       // file name (variable size)
                                       // extra field (variable size)
                                       // file comment (variable size)
};

struct LocalFileHeader
{
    uint32_t signature;               // local file header signature
    uint16_t versionToExtract;        // version needed to extract
    uint16_t generalPurposeBitFlag;   // general purpose bit flag
    uint16_t compressionMethod;       // compression method
    uint16_t modificationTime;        // last mod file time
    uint16_t modificationDate;        // last mod file date
    uint32_t crc32;                   // crc-32
    uint32_t compressedSize;          // compressed size
    uint32_t uncompressedSize;        // uncompressed size
    uint16_t filenameLength;          // file name length
    uint16_t extraFieldLength;        // extra field length
                                      // file name (variable size)
                                      // extra field (variable size)
};

#pragma pack(pop)

#endif   // ZIP_H_INCLUDED
