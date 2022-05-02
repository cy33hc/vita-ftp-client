#include <string.h>
#include "fs.h"
#include "config.h"
#include "windows.h"
#include "ftpclient.h"
#include "util.h"

namespace Actions {
    
    void RefreshLocalFiles(bool apply_filter)
    {
        Windows::LockLocal();
        multi_selected_local_files.clear();
        local_files.clear();
        int err;
        if (strlen(local_filter)>0 && apply_filter)
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
        Windows::UnlockLocal();
        if (err != 0)
            sprintf(status_message, "Failed to read contents of directory \"%s\" or folder does not exists.", local_directory);
    }

    void RefreshRemoteFiles(bool apply_filter)
    {
        if (!ftpclient->Noop())
        {
            ftpclient->Quit();
            sprintf(status_message, "426 Connection closed");
            return;
        }

        Windows::LockRemote();
        multi_selected_remote_files.clear();
        remote_files.clear();
        if (strlen(remote_filter)>0 && apply_filter)
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
        Windows::UnlockRemote();
        sprintf(status_message, "%s", ftpclient->LastResponse());
    }

    void HandleChangeLocalDirectory(const FsEntry entry)
    {
        if (!entry.isDir)
            return;

        if (strcmp(entry.name, "..") == 0)
        {
            std::string temp_path = std::string(entry.directory);
            sprintf(local_directory, "%s", temp_path.substr(0, temp_path.find_last_of("/")).c_str());
            sprintf(local_file_to_select, "%s", temp_path.substr(temp_path.find_last_of("/")+1).c_str());
        }
        else
        {
            sprintf(local_directory, "%s", entry.path);
        }
        RefreshLocalFiles(false);
        if (strcmp(entry.name, "..") != 0)
        {
            sprintf(local_file_to_select, "%s", local_files[0].name);
        }
        selected_local_file = nullptr;
        selected_action = ACTION_NONE;
    }

    void HandleChangeRemoteDirectory(const FsEntry entry)
    {
        if (!entry.isDir)
            return;
            
        if (strcmp(entry.name, "..") == 0)
        {
            std::string temp_path = std::string(entry.directory);
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
            sprintf(remote_directory, "%s", entry.path);
        }
        RefreshRemoteFiles(false);
        if (strcmp(entry.name, "..") != 0)
        {
            sprintf(remote_file_to_select, "%s", remote_files[0].name);
        }
        selected_remote_file = nullptr;
        selected_action = ACTION_NONE;
    }

    void HandleRefreshLocalFiles()
    {
        int prev_count = local_files.size();
        RefreshLocalFiles(false);
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
        RefreshRemoteFiles(false);
        int new_count = remote_files.size();
        if (prev_count != new_count)
        {
            sprintf(remote_file_to_select, "%s", remote_files[0].name);
        }
        selected_action = ACTION_NONE;
    }

    void CreateNewLocalFolder(char *new_folder)
    {
        std::string folder = std::string(new_folder);
        folder = Util::Rtrim(Util::Trim(folder, " "), "/");
        std::string path = FS::GetPath(local_directory, folder);
        FS::MkDirs(path);
        RefreshLocalFiles(false);
        sprintf(local_file_to_select, "%s", folder.c_str());
    }

    void CreateNewRemoteFolder(char *new_folder)
    {
        std::string folder = std::string(new_folder);
        folder = Util::Rtrim(Util::Trim(folder, " "), "/");
        std::string path = FS::GetPath(remote_directory, folder);
        ftpclient->Mkdir(path.c_str());
        RefreshRemoteFiles(false);
        sprintf(remote_file_to_select, "%s", folder.c_str());
    }

    void RenameLocalFolder(char *old_path, char *new_path)
    {
        std::string new_name = std::string(new_path);
        new_name = Util::Rtrim(Util::Trim(new_name, " "), "/");
        std::string path = FS::GetPath(local_directory, new_name);
        FS::Rename(old_path, path);
        RefreshLocalFiles(false);
        sprintf(local_file_to_select, "%s", new_name.c_str());
    }

    void RenameRemoteFolder(char *old_path, char *new_path)
    {
        std::string new_name = std::string(new_path);
        new_name = Util::Rtrim(Util::Trim(new_name, " "), "/");
        std::string path = FS::GetPath(remote_directory, new_name);
        ftpclient->Rename(old_path, path.c_str());
        RefreshRemoteFiles(false);
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
        bk_activity_thid = sceKernelCreateThread("delete_local_files_thread", (SceKernelThreadEntry)DeleteSelectedLocalFilesThread, 0x10000100, 0x4000, 0, 0, NULL);
		if (bk_activity_thid >= 0)
			sceKernelStartThread(bk_activity_thid, 0, NULL);
    }

    int DeleteSelectedRemotesFilesThread(SceSize args, void *argp)
    {
        if (ftpclient->Noop())
        {
            for (std::set<FsEntry>::iterator it = multi_selected_remote_files.begin(); it != multi_selected_remote_files.end(); ++it)
            {
                if (it->isDir)
                    ftpclient->Rmdir(it->path, true);
                else
                    ftpclient->Delete(it->path);
            }
            selected_action = ACTION_REFRESH_REMOTE_FILES;
        }
        else
        {
            ftpclient->Quit();
            sprintf(status_message, "426 Connection closed");
            DisconnectFTP();
        }
        activity_inprogess = false;
        Windows::SetModalMode(false);
        return sceKernelExitDeleteThread(0);
    }

    void DeleteSelectedRemotesFiles()
    {
        bk_activity_thid = sceKernelCreateThread("delete_remote_files_thread", (SceKernelThreadEntry)DeleteSelectedRemotesFilesThread, 0x10000100, 0x4000, 0, 0, NULL);
		if (bk_activity_thid >= 0)
			sceKernelStartThread(bk_activity_thid, 0, NULL);
    }

    int UploadFile(const char *src, const char* dest)
    {
        int ret;
        int64_t filesize;
        ret = ftpclient->Noop();
        if (ret == 0)
        {
            ftpclient->Quit();
            sprintf(status_message, "426 Connection closed");
            return ret;
        }
            
        if (overwrite_type == OVERWRITE_PROMPT && ftpclient->Size(dest, &filesize, FtpClient::transfermode::image))
        {
            sprintf(confirm_message, "Overwrite %s?", dest);
            confirm_state = CONFIRM_WAIT;
            action_to_take = selected_action;
            activity_inprogess = false;
            while (confirm_state == CONFIRM_WAIT)
            {
                sceKernelDelayThread(100000);
            }
            activity_inprogess = true;
            selected_action = action_to_take;
        }
        else if (overwrite_type == OVERWRITE_NONE && ftpclient->Size(dest, &filesize, FtpClient::transfermode::image))
        {
            confirm_state = CONFIRM_NO;
        }
        else
        {
            confirm_state = CONFIRM_YES;
        }

        if (confirm_state == CONFIRM_YES)
        {
            return ftpclient->Put(src, dest, FtpClient::transfermode::image, 0);
        }

        return 1;
    }

    int Upload(const FsEntry &src, const char *dest)
    {
        if (stop_activity)
            return 1;

        int ret;
        if (src.isDir)
        {
            int err;
            std::vector<FsEntry> entries = FS::ListDir(src.path, &err);
            ftpclient->Mkdir(dest);
            for (int i=0; i<entries.size(); i++)
            {
                if (stop_activity)
		            return 1;

                int path_length = strlen(dest) + strlen(entries[i].name) + 2;
                char *new_path = malloc(path_length);
                snprintf(new_path, path_length, "%s%s%s", dest, FS::hasEndSlash(dest) ? "" : "/", entries[i].name);

                if (entries[i].isDir)
                {
                    if (strcmp(entries[i].name, "..") == 0)
                        continue;

                    ftpclient->Mkdir(new_path);
                    ret = Upload(entries[i], new_path);
                    if (ret <= 0)
                    {
                        free(new_path);
                        return ret;
                    }
                }
                else
                {
                    snprintf(activity_message, 1024, "Uploading %s", entries[i].path);
                    bytes_to_download = entries[i].file_size;
                    ret = UploadFile(entries[i].path, new_path);
                    if (ret <= 0)
                    {
                        sprintf(status_message, "Failed to upload file %s", entries[i].path);
                        free(new_path);
                        return ret;
                    }
                }
                free(new_path);
            }
        }
        else
        {
            int path_length = strlen(dest) + strlen(src.name) + 2;
            char *new_path = malloc(path_length);
            snprintf(new_path, path_length, "%s%s%s", dest, FS::hasEndSlash(dest) ? "" : "/", src.name);
            snprintf(activity_message, 1024, "Uploading %s", src.name);
            bytes_to_download = src.file_size;
            ret = UploadFile(src.path, new_path);
            if (ret <= 0)
            {
                free(new_path);
                sprintf(status_message, "Failed to upload file %s", src.name);
                return 0;
            }
            free(new_path);
        }
        return 1;
    }

    int UploadFilesThread(SceSize args, void *argp)
    {
        file_transfering = true;
        for (std::set<FsEntry>::iterator it = multi_selected_local_files.begin(); it != multi_selected_local_files.end(); ++it)
        {
            if (it->isDir)
            {
                char new_dir[512];
                sprintf(new_dir, "%s%s%s", remote_directory, FS::hasEndSlash(remote_directory)? "" : "/", it->name);
                Upload(*it, new_dir);
            }
            else
            {
                Upload(*it, remote_directory);
            }
        }
        activity_inprogess = false;
        file_transfering = false;
        multi_selected_local_files.clear();
        Windows::SetModalMode(false);
        selected_action = ACTION_REFRESH_REMOTE_FILES;
        return sceKernelExitDeleteThread(0);
    }
    
    void UploadFiles()
    {
        bk_activity_thid = sceKernelCreateThread("upload_files_thread", (SceKernelThreadEntry)UploadFilesThread, 0x10000100, 0x4000, 0, 0, NULL);
		if (bk_activity_thid >= 0)
			sceKernelStartThread(bk_activity_thid, 0, NULL);
    }

    int DownloadFile(const char *src, const char* dest)
    {
        int ret;
        ret = ftpclient->Size(src, &bytes_to_download, FtpClient::transfermode::image);
        if (ret == 0)
        {
            sprintf(status_message, "426 Connection closed");
            return ret;
        }

        if (overwrite_type == OVERWRITE_PROMPT && FS::FileExists(dest))
        {
            sprintf(confirm_message, "Overwrite %s?", dest);
            confirm_state = CONFIRM_WAIT;
            action_to_take = selected_action;
            activity_inprogess = false;
            while (confirm_state == CONFIRM_WAIT)
            {
                sceKernelDelayThread(100000);
            }
            activity_inprogess = true;
            selected_action = action_to_take;
        }
        else if (overwrite_type == OVERWRITE_NONE && FS::FileExists(dest))
        {
            confirm_state = CONFIRM_NO;
        }
        else
        {
            confirm_state = CONFIRM_YES;
        }

        if (confirm_state == CONFIRM_YES)
        {
            return ftpclient->Get(dest, src, FtpClient::transfermode::image, 0);
        }

        return 1;
    }

    int Download(const FsEntry &src, const char *dest)
    {
        if (stop_activity)
            return 1;

        int ret;
        if (src.isDir)
        {
            int err;
            std::vector<FsEntry> entries = ftpclient->ListDir(src.path);
            FS::MkDirs(dest);
            for (int i=0; i<entries.size(); i++)
            {
                if (stop_activity)
		            return 1;

                int path_length = strlen(dest) + strlen(entries[i].name) + 2;
                char *new_path = malloc(path_length);
                snprintf(new_path, path_length, "%s%s%s", dest, FS::hasEndSlash(dest) ? "" : "/", entries[i].name);

                if (entries[i].isDir)
                {
                    if (strcmp(entries[i].name, "..") == 0)
                        continue;

                    FS::MkDirs(new_path);
                    ret = Download(entries[i], new_path);
                    if (ret <= 0)
                    {
                        free(new_path);
                        return ret;
                    }
                }
                else
                {
                    snprintf(activity_message, 1024, "Downloading %s", entries[i].path);
                    ret = DownloadFile(entries[i].path, new_path);
                    if (ret <= 0)
                    {
                        sprintf(status_message, "Failed to downloadload file %s", entries[i].path);
                        free(new_path);
                        return ret;
                    }
                }
                free(new_path);
            }
        }
        else
        {
            int path_length = strlen(dest) + strlen(src.name) + 2;
            char *new_path = malloc(path_length);
            snprintf(new_path, path_length, "%s%s%s", dest, FS::hasEndSlash(dest) ? "" : "/", src.name);
            snprintf(activity_message, 1024, "Downloading %s", src.path);
            ret = DownloadFile(src.path, new_path);
            if (ret <= 0)
            {
                free(new_path);
                sprintf(status_message, "Failed to download file %s", src.path);
                return 0;
            }
            free(new_path);
        }
        return 1;
    }

    int DownloadFilesThread(SceSize args, void *argp)
    {
        file_transfering = true;
        for (std::set<FsEntry>::iterator it = multi_selected_remote_files.begin(); it != multi_selected_remote_files.end(); ++it)
        {
            if (it->isDir)
            {
                char new_dir[512];
                sprintf(new_dir, "%s%s%s", local_directory, FS::hasEndSlash(local_directory)? "" : "/", it->name);
                Download(*it, new_dir);
            }
            else
            {
                Download(*it, local_directory);
            }
        }
        file_transfering = false;
        activity_inprogess = false;
        Windows::LockRemote();
        multi_selected_remote_files.clear();
        Windows::UnlockRemote();
        Windows::SetModalMode(false);
        selected_action = ACTION_REFRESH_LOCAL_FILES;
        return sceKernelExitDeleteThread(0);
    }
    
    void DownloadFiles()
    {
        bk_activity_thid = sceKernelCreateThread("download_files_thread", (SceKernelThreadEntry)DownloadFilesThread, 0x10000100, 0x4000, 0, 0, NULL);
		if (bk_activity_thid >= 0)
			sceKernelStartThread(bk_activity_thid, 0, NULL);
    }

    int KeepAliveThread(SceSize args, void *argp)
    {
        SceUInt64 idle;
        while (true)
        {
            idle = ftpclient->GetIdleTime();
            if (idle > 60000000)
            {
                if (!ftpclient->Noop())
                {
                    ftpclient->Quit();
                    sprintf(status_message, "426 Remote Server has terminated the connection");
                    sceKernelExitDeleteThread(0);
                }
            }
            sceKernelDelayThread(500000);
        }
    }

    void ConnectFTP()
    {
        CONFIG::SaveConfig();
        if (ftpclient->Connect(ftp_settings.server_ip, ftp_settings.server_port))
        {
            if (ftpclient->Login(ftp_settings.username, ftp_settings.password))
            {
                RefreshRemoteFiles(false);
                sprintf(status_message, "%s", ftpclient->LastResponse());

                ftp_keep_alive_thid  = sceKernelCreateThread("ftp_keep_alive_thread", (SceKernelThreadEntry)KeepAliveThread, 0x10000100, 0x4000, 0, 0, NULL);
		        if (ftp_keep_alive_thid >= 0)
			        sceKernelStartThread(ftp_keep_alive_thid, 0, NULL);
            }
            else
            {
                sprintf(status_message, "300 Failed Login. Please check your username or password.");
            }
        }
        else
        {
            sprintf(status_message, "426 Failed. Connection timeout.");
        }
        selected_action = ACTION_NONE;
    }

    void DisconnectFTP()
    {
        ftpclient->Quit();
        selected_remote_file = nullptr;
        multi_selected_remote_files.clear();
        remote_files.clear();
        sprintf(remote_directory, "/");
        sprintf(status_message, "");
    }

    void SelectAllLocalFiles()
    {
        for (int i=0; i<local_files.size(); i++)
        {
            if (strcmp(local_files[i].name, "..") != 0)
                multi_selected_local_files.insert(local_files[i]);
        }
    }

    void SelectAllRemoteFiles()
    {
         for (int i=0; i<remote_files.size(); i++)
        {
            if (strcmp(remote_files[i].name, "..") != 0)
                multi_selected_remote_files.insert(remote_files[i]);
        }
    }
}
