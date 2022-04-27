#ifndef ACTIONS_H
#define ACTIONS_H

#include "fs.h"

enum ACTIONS {
    ACTION_NONE = 0,
    ACTION_UPLOAD,
    ACTION_DOWNLOAD,
    ACTION_DELETE_LOCAL,
    ACTION_DELETE_REMOTE,
    ACTION_RENAME_LOCAL,
    ACTION_RENAME_REMOTE,
    ACTION_NEW_LOCAL_FOLDER,
    ACTION_NEW_REMOTE_FOLDER,
    ACTION_CHANGE_LOCAL_DIRECTORY,
    ACTION_CHANGE_REMOTE_DIRECTORY,
    ACTION_CLEAR_LOCAL_FILTER,
    ACTION_CLEAR_REMOTE_FILTER,
    ACTION_REFRESH_LOCAL_FILES,
    ACTION_REFRESH_REMOTE_FILES,
    ACTION_SHOW_LOCAL_PROPERTIES,
    ACTION_SHOW_REMOTE_PROPERTIES,
    ACTION_CONNECT_FTP,
    ACTION_DISCONNECT_FTP
};

static SceUID bk_activity_thid = -1;

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
    void RenameLocalFolder(char *old_path, char *new_path);
    void RenameRemoteFolder(char *old_path, char *new_path);
    int DeleteSelectedLocalFilesThread(SceSize args, void *argp);
    void DeleteSelectedLocalFiles();
    int DeleteSelectedRemotesFilesThread(SceSize args, void *argp);
    void DeleteSelectedRemotesFiles();
    int UploadFilesThread(SceSize args, void *argp);
    void UploadFiles();
    int  DownloadFilesThread(SceSize args, void *argp);
    void DownloadFiles();
    void ConnectFTP();
}

#endif