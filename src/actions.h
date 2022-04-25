#ifndef ACTIONS_H
#define ACTIONS_H

#include "fs.h"

enum ACTIONS {
    ACTION_NONE = 0,
    ACTION_CHANGE_LOCAL_DIRECTORY,
    ACTION_COPY_LOCAL,
    ACTION_PASTE_LOCAL,
    ACTION_DELETE_LOCAL,
    ACTION_CLEAR_LOCAL_FILTER,
    ACTION_REFRESH_LOCAL_FILES,
    ACTION_CHANGE_REMOTE_DIRECTORY,
    ACTION_COPY_REMOTE,
    ACTION_PASTE_REMOTE,
    ACTION_DELETE_REMOTE,
    ACTION_CLEAR_REMOTE_FILTER,
    ACTION_REFRESH_REMOTE_FILES,
    ACTION_CONNECT_FTP
};

enum CopyType {
    COPY_TYPE_NONE = 0,
    COPY_TYPE_LOCAL_ENTRY,
    COPY_TYPE_REMOTE_ENTRY
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