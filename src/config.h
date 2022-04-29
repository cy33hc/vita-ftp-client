#ifndef LAUNCHER_CONFIG_H
#define LAUNCHER_CONFIG_H

#include <string>
#include <vector>
#include <algorithm>
#include "fs.h"

#define APP_ID "FTPCLI001"
#define DATA_PATH "ux0:data/" APP_ID
#define APP_PATH "ux0:app/" APP_ID
#define CONFIG_INI_FILE  DATA_PATH "/config.ini"

#define FTP_CLIENT_VPK_URL "https://github.com/cy33hc/vita-ftp-client/releases/download/latest/ftpclient.vpk"
#define FTP_CLIENT_VERSION_URL "https://github.com/cy33hc/vita-ftp-client/releases/download/latest/version.txt"
#define FTP_CLIENT_VERSION_PATH APP_PATH "/version.txt"
#define FTP_CLIENT_VERSION_UPDATE_PATH DATA_PATH "/tmp/version.txt"
#define FTP_CLIENT_VPK_UPDATE_PATH DATA_PATH "/tmp/ftpclient.vpk"

#define CONFIG_GLOBAL "Global"

#define CONFIG_STYLE_NAME "style"
#define CONFIG_DEFAULT_STYLE_NAME "Default"
#define CONFIG_SWAP_XO "swap_xo"

#define CONFIG_BACKGROUD_MUSIC "backgroud_music"
#define CONFIG_ENABLE_BACKGROUND_MUSIC "enable_backgroud_music"

#define CONFIG_FTP_SERVER_IP "ftp_server_ip"
#define CONFIG_FTP_SERVER_PORT "ftp_server_port"
#define CONFIG_FTP_SERVER_USER "ftp_server_user"
#define CONFIG_FTP_SERVER_PASSWORD "ftp_server_password"
#define CONFIG_FTP_TRANSFER_MODE "ftp_transfer_mode"

#define CONFIG_LOCAL_DIRECTORY "local_directory"
#define CONFIG_REMOTE_DIRECTORY "remote_directory"

struct FtpSettings {
    char server_ip[16];
    char username[33];
    char password[25];
    int server_port;
    bool pasv_mode;
};

extern bool swap_xo;
extern std::vector<std::string> bg_music_list;
extern bool enable_backgrou_music;
extern FtpSettings ftp_settings;
extern char local_directory[];
extern char remote_directory[];

namespace CONFIG {
    void LoadConfig();
    void SaveConfig();
    void RemoveFromMultiValues(std::vector<std::string> &multi_values, std::string value);
    void ParseMultiValueString(const char* prefix_list, std::vector<std::string> &prefixes, bool toLower);
    std::string GetMultiValueString(std::vector<std::string> &multi_values);
}
#endif
