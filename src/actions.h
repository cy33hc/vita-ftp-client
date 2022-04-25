#ifndef ACTIONS_H
#define ACTIONS_H

#include "fs.h"

enum ACTIONS {
    NONE = 0,
    CHANGE_LOCAL_DIRECTORY,
    COPY_LOCAL,
    PASTE_LOCAL,
    DELETE_LOCAL,
    CLEAR_LOCAL_FILTER,
    REFRESH_LOCAL_FILES,
    CHANGE_REMOTE_DIRECTORY,
    COPY_REMOTE,
    PASTE_REMOTE,
    DELETE_REMOTE,
    CLEAR_REMOTE_FILTER,
    REFRESH_REMOTE_FILES,
    CONNECT_FTP
};

enum CopyType {
    SINGLE_LOCAL_ENTRY = 0,
    MULTIPLE_LOCAL_ENTRIES,
    SINGLE_REMOTE_ENTRY,
    MULTIPLE_REMOTE_ENTRIES
};

struct CopyStruct {
    void *from_entry;
    CopyType from_type;
    void *to_entry;
    CopyType to_type;
};

namespace Actions {

    void RefreshLocalFiles();
    void RefreshRemoteFiles();
    void HandleChangeLocalDirectory(FsEntry *entry);
    void HandleChangeRemoteDirectory(FtpDirEntry *entry);
    void HandleRefreshLocalFiles();
    void HandleRefreshRemoteFiles();
    void HandleClearLocalFilter();
    void HandleClearRemoteFilter();
    void ConnectFTP();
}

#endif