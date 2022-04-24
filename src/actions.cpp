#include "fs.h"
#include "config.h"
#include "windows.h"
#include "ftpclient.h"

namespace Actions {
    
    void HandleChangeLocalDirectory(FsEntry *entry)
    {
        if (!entry->isDir)
            return;

        if (strcmp(entry->name, "..") == 0)
        {
            std::string temp_path = std::string(entry->directory);
            sprintf(local_directory, "%s", temp_path.substr(0, temp_path.find_last_of("/")).c_str());
            sprintf(local_file_to_select, "%s", temp_path.substr(temp_path.find_last_of("/")+1).c_str());
        }
        else
        {
            sprintf(local_directory, "%s", entry->path);
            sprintf(local_file_to_select, "..");
        }
        local_files.clear();
        local_files = FS::ListDir(local_directory);
        FS::Sort(local_files);
        selected_local_file = nullptr;
        selected_action = NONE;
    }

    void HandleChangeRemoteDirectory(FtpDirEntry *entry)
    {
        if (!entry->isDir)
            return;
            
        if (strcmp(entry->name, "..") == 0)
        {
            std::string temp_path = std::string(entry->directory);
            if (temp_path.size()>1)
            {
                if (temp_path.find_last_of("/") == 0)
                {
                    sprintf(remote_directory, "/");
                }
                else
                {
                    sprintf(remote_directory, "%s", temp_path.substr(0, temp_path.find_last_of("/")).c_str());
                }
            }
            sprintf(remote_file_to_select, "%s", temp_path.substr(temp_path.find_last_of("/")+1).c_str());
        }
        else
        {
            sprintf(remote_directory, "%s", entry->path);
            sprintf(remote_file_to_select, "..");
        }
        remote_files.clear();
        remote_files = ftpclient->ListDir(remote_directory);
        sprintf(status_message, "%s", ftpclient->LastResponse());
        FtpClient::Sort(remote_files);
        selected_remote_file = nullptr;
        selected_action = NONE;
    }

    void ConnectFTP()
    {
        if (ftpclient->Connect(ftp_settings.server_ip, ftp_settings.server_port))
        {
            if (ftpclient->Login(ftp_settings.username, ftp_settings.password))
            {
                remote_files.clear();
                remote_files = ftpclient->ListDir(remote_directory);
                FtpClient::Sort(remote_files);
                sprintf(status_message, "%s", ftpclient->LastResponse());
            }
            else
            {
                snprintf(status_message, 1024, "%s", ftpclient->LastResponse());
            }
        }
        else
        {
            snprintf(status_message, 1024, "%s", ftpclient->LastResponse());
        }
        selected_action = NONE;
    }
}
