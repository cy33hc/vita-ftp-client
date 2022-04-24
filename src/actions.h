#ifndef ACTIONS_H
#define ACTIONS_H

#include "fs.h"

enum ACTIONS {
    NONE = 0,
    CHANGE_LOCAL_DIRECTORY,
    COPY_LOCAL,
    PASTE_LOCAL,
    DELETE_LOCAL,
    CHANGE_REMOTE_DIRECTORY,
    COPY_REMOTE,
    PASTE_REMOTE,
    DELETE_REMOTE,
    CONNECT_FTP
};

namespace Actions {

    void HandleChangeLocalDirectory(FsEntry *entry);
    void HandleChangeRemoteDirectory(FtpDirEntry *entry);
    void ConnectFTP();
}

#endif