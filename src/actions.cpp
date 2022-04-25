#include <string.h>
#include "fs.h"
#include "config.h"
#include "windows.h"
#include "ftpclient.h"
#include "util.h"

namespace Actions {
    
    void RefreshLocalFiles()
    {
        multi_selected_local_files.clear();
        local_files.clear();
        int err;
        local_files = FS::ListDir(local_directory, &err);
        if (strlen(local_filter)>0)
        {
            std::string lower_filter = Util::ToLower(local_filter);
            for (std::vector<FsEntry>::iterator it=local_files.begin(); it!=local_files.end(); )
            {
                std::string lower_name = Util::ToLower(it->name);
                if (lower_name.find(lower_filter) == std::string::npos && strcmp(it->name, "..") != 0)
                {
                    local_files.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
        FS::Sort(local_files);
        if (err != 0)
            sprintf(status_message, "Failed to read contents of directory \"%s\" or folder does not exists.", local_directory);
    }

    void RefreshRemoteFiles()
    {
        if (!ftpclient->IsConnected())
        {
            return;
        }

        multi_selected_remote_files.clear();
        remote_files.clear();
        remote_files = ftpclient->ListDir(remote_directory);
        sprintf(status_message, "%s", ftpclient->LastResponse());
        if (strlen(remote_filter)>0)
        {
            std::string lower_filter = Util::ToLower(remote_filter);
            for (std::vector<FtpDirEntry>::iterator it=remote_files.begin(); it!=remote_files.end(); )
            {
                std::string lower_name = Util::ToLower(it->name);
                if (lower_name.find(lower_filter) == std::string::npos && strcmp(it->name, "..") != 0)
                {
                    remote_files.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
        FtpClient::Sort(remote_files);
    }

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
        }
        RefreshLocalFiles();
        if (strcmp(entry->name, "..") != 0)
        {
            sprintf(local_file_to_select, "%s", local_files[0].name);
        }
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
        }
        RefreshRemoteFiles();
        if (strcmp(entry->name, "..") != 0)
        {
            sprintf(remote_file_to_select, "%s", remote_files[0].name);
        }
        selected_remote_file = nullptr;
        selected_action = NONE;
    }

    void HandleRefreshLocalFiles()
    {
        int prev_count = local_files.size();
        RefreshLocalFiles();
        int new_count = local_files.size();
        if (prev_count != new_count)
        {
            sprintf(local_file_to_select, "%s", local_files[0].name);
        }
        selected_action = NONE;
    }

    void HandleRefreshRemoteFiles()
    {
        int prev_count = remote_files.size();
        RefreshRemoteFiles();
        int new_count = remote_files.size();
        if (prev_count != new_count)
        {
            sprintf(remote_file_to_select, "%s", remote_files[0].name);
        }
        selected_action = NONE;
    }

    void HandleClearLocalFilter()
    {
        sprintf(local_filter, "");
        HandleRefreshLocalFiles();
    }

    void HandleClearRemoteFilter()
    {
        sprintf(remote_filter, "");
        HandleRefreshRemoteFiles();
    }

    void ConnectFTP()
    {
        CONFIG::SaveConfig();
        if (ftpclient->Connect(ftp_settings.server_ip, ftp_settings.server_port))
        {
            if (ftpclient->Login(ftp_settings.username, ftp_settings.password))
            {
                RefreshRemoteFiles();
                sprintf(status_message, "%s", ftpclient->LastResponse());
            }
            else
            {
                sprintf(status_message, "300 Failed Login. Please check your username or password.");
            }
        }
        else
        {
            sprintf(status_message, "300 Failed. Connection timeout.");
        }
        selected_action = NONE;
    }
}
