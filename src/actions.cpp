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
        if (strlen(local_filter)>0)
        {
            std::vector<FsEntry> temp_files = FS::ListDir(local_directory, &err);
            std::string lower_filter = Util::ToLower(local_filter);
            for (std::vector<FsEntry>::iterator it=temp_files.begin(); it!=temp_files.end(); )
            {
                std::string lower_name = Util::ToLower(it->name);
                if (lower_name.find(lower_filter) != std::string::npos || strcmp(it->name, "..") == 0)
                {
                    local_files.push_back(*it);
                }
                ++it;
            }
            temp_files.clear();
        }
        else
        {
            local_files = FS::ListDir(local_directory, &err);
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
        if (strlen(remote_filter)>0)
        {
            std::vector<FsEntry> temp_files = ftpclient->ListDir(remote_directory);
            std::string lower_filter = Util::ToLower(remote_filter);
            for (std::vector<FsEntry>::iterator it=temp_files.begin(); it!=temp_files.end(); )
            {
                std::string lower_name = Util::ToLower(it->name);
                if (lower_name.find(lower_filter) != std::string::npos || strcmp(it->name, "..") == 0)
                {
                    remote_files.push_back(*it);
                }
                ++it;
            }
            temp_files.clear();
        }
        else
        {
            remote_files = ftpclient->ListDir(remote_directory);
        }
        FS::Sort(remote_files);
        sprintf(status_message, "%s", ftpclient->LastResponse());
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
        selected_action = ACTION_NONE;
    }

    void HandleChangeRemoteDirectory(FsEntry *entry)
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
        selected_action = ACTION_NONE;
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
        selected_action = ACTION_NONE;
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
        selected_action = ACTION_NONE;
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

    void CreateNewLocalFolder(char *new_folder)
    {
        std::string folder = std::string(new_folder);
        folder = Util::Rtrim(Util::Trim(folder, " "), "/");
        std::string path = FS::GetPath(local_directory, folder);
        FS::MkDirs(path);
        RefreshLocalFiles();
        sprintf(local_file_to_select, "%s", folder.c_str());
    }

    void CreateNewRemoteFolder(char *new_folder)
    {
        std::string folder = std::string(new_folder);
        folder = Util::Rtrim(Util::Trim(folder, " "), "/");
        std::string path = FS::GetPath(remote_directory, folder);
        ftpclient->Mkdir(path.c_str());
        RefreshRemoteFiles();
        sprintf(remote_file_to_select, "%s", folder.c_str());
    }

    void RenameLocalFolder(char *old_path, char *new_path)
    {
        std::string new_name = std::string(new_path);
        new_name = Util::Rtrim(Util::Trim(new_name, " "), "/");
        std::string path = FS::GetPath(local_directory, new_name);
        FS::Rename(old_path, path);
        RefreshLocalFiles();
        sprintf(local_file_to_select, "%s", new_name.c_str());
    }

    void RenameRemoteFolder(char *old_path, char *new_path)
    {
        std::string new_name = std::string(new_path);
        new_name = Util::Rtrim(Util::Trim(new_name, " "), "/");
        std::string path = FS::GetPath(remote_directory, new_name);
        ftpclient->Rename(old_path, path.c_str());
        RefreshRemoteFiles();
        sprintf(remote_file_to_select, "%s", new_name.c_str());
    }

    int DeleteSelectedLocalFilesThread(SceSize args, void *argp)
    {
        for (std::set<FsEntry>::iterator it = multi_selected_local_files.begin(); it != multi_selected_local_files.end(); ++it)
        {
            FS::RmRecursive(it->path);
        }
        activity_inprogess = false;
        Windows::SetModalMode(false);
        selected_action = ACTION_REFRESH_LOCAL_FILES;
        return sceKernelExitDeleteThread(0);
    }

    void DeleteSelectedLocalFiles()
    {
        delete_files_thid = sceKernelCreateThread("delete_files_thread", (SceKernelThreadEntry)DeleteSelectedLocalFilesThread, 0x10000100, 0x4000, 0, 0, NULL);
		if (delete_files_thid >= 0)
			sceKernelStartThread(delete_files_thid, 0, NULL);
    }

    int DeleteSelectedRemotesFilesThread(SceSize args, void *argp)
    {
        for (std::set<FsEntry>::iterator it = multi_selected_remote_files.begin(); it != multi_selected_remote_files.end(); ++it)
        {
            if (it->isDir)
                ftpclient->Rmdir(it->path, true);
            else
                ftpclient->Delete(it->path);
        }
        activity_inprogess = false;
        Windows::SetModalMode(false);
        selected_action = ACTION_REFRESH_REMOTE_FILES;
        return sceKernelExitDeleteThread(0);
    }

    void DeleteSelectedRemotesFiles()
    {
        delete_files_thid = sceKernelCreateThread("delete_files_thread", (SceKernelThreadEntry)DeleteSelectedRemotesFilesThread, 0x10000100, 0x4000, 0, 0, NULL);
		if (delete_files_thid >= 0)
			sceKernelStartThread(delete_files_thid, 0, NULL);
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
        selected_action = ACTION_NONE;
    }
}
