#ifndef ACTIONS_H
#define ACTIONS_H

#include "fs.h"

enum ACTIONS {
    ACTION_NONE = 0,
    ACTION_CHANGE_LOCAL_DIRECTORY,
    ACTION_COPY,
    ACTION_PASTE,
    ACTION_DELETE,
    ACTION_RENAME,
    ACTION_NEW_LOCAL_FOLDER,
    ACTION_NEW_REMOTE_FOLDER,
    ACTION_CLEAR_LOCAL_FILTER,
    ACTION_REFRESH_LOCAL_FILES,
    ACTION_CHANGE_REMOTE_DIRECTORY,
    ACTION_CLEAR_REMOTE_FILTER,
    ACTION_REFRESH_REMOTE_FILES,
    ACTION_CONNECT_FTP
};

enum CopyType {
    COPY_TYPE_NONE = 0,
    COPY_TYPE_LOCAL_ENTRY,
    COPY_TYPE_REMOTE_ENTRY
};

struct CopyStruct {
    std::set<FsEntry> files;
    CopyType type;
};

namespace Actions {

    void RefreshLocalFiles();
    void RefreshRemoteFiles();
    void HandleChangeLocalDirectory(FsEntry *entry);
    void HandleChangeRemoteDirectory(FsEntry *entry);
    void HandleRefreshLocalFiles();
    void HandleRefreshRemoteFiles();
    void HandleClearLocalFilter();
    void HandleClearRemoteFilter();
    void CreateNewLocalFolder(char *new_folder);
    void CreateNewRemoteFolder(char *new_folder);
    void ConnectFTP();
}

#endif