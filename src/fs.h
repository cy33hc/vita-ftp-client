#ifndef LAUNCHER_FS_H
#define LAUNCHER_FS_H

#pragma once
#include <string.h>
#include <string>
#include <vector>

#include <cstdint>
#define MAX_PATH_LENGTH 1024

/**
  * @brief Date and time representation
**/  
typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t dayOfWeek;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint32_t microsecond;
} DateTime;

struct FsEntry {
    char directory[512];
    char name[256];
    char display_size[32];
    char path[1024];
    int64_t file_size;
    bool isDir;
    DateTime modified;

    friend bool operator<(FsEntry const& a, FsEntry const& b)
    {
        return strcmp(a.name, b.name) < 0;
    }

};

namespace FS {
    std::string GetPath(const std::string& path1, const std::string& path2);

    void MkDirs(const std::string& path, bool prev=false);
    void MkPrevDirs(const std::string& path);

    void Rm(const std::string& file);
    void RmDir(const std::string& path);
    int RmRecursive(const std::string& path);

    int64_t GetSize(const char* path);

    bool FileExists(const std::string& path);
    bool FolderExists(const std::string& path);
    
    void Rename(const std::string& from, const std::string& to);

    // creates file (if it exists, truncates size to 0)
    void* Create(const std::string& path);

    // open existing file in read/write, fails if file does not exist
    void* OpenRW(const std::string& path);
 
    // open existing file in read/write, fails if file does not exist
    void* OpenRead(const std::string& path);

    // open file for writing, next write will append data to end of it
    void* Append(const std::string& path);

    void Close(void* f);

    int64_t Seek(void* f, uint64_t offset);
    int Read(void* f, void* buffer, uint32_t size);
    int Write(void* f, const void* buffer, uint32_t size);

    std::vector<char> Load(const std::string& path);
    void Save(const std::string& path, const void* data, uint32_t size);

    std::vector<std::string> ListFiles(const std::string& path);
    std::vector<FsEntry> ListDir(const std::string& path, int *err);

    void Sort(std::vector<FsEntry> &list);

    int hasEndSlash(const char *path);
}

#endif