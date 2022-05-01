#include <vitasdk.h>
#include <string>
#include <cstring>
#include <map>

#include "config.h"
#include "fs.h"
#include "style.h"


extern "C" {
	#include "inifile.h"
}

bool swap_xo;
std::vector<std::string> bg_music_list;
bool enable_backgrou_music;
FtpSettings ftp_settings;
char local_directory[MAX_PATH_LENGTH];
char remote_directory[MAX_PATH_LENGTH];
char app_ver[6];

namespace CONFIG {

    void LoadConfig()
    {
        const char* bg_music_list_str;

        if (!FS::FolderExists(DATA_PATH))
        {
            FS::MkDirs(DATA_PATH);
        }

		OpenIniFile (CONFIG_INI_FILE);

        // Load styles
        if (!FS::FolderExists(STYLES_FOLDER))
        {
            FS::MkDirs(STYLES_FOLDER);
        }

        char* style_value = ReadString(CONFIG_GLOBAL, CONFIG_STYLE_NAME, CONFIG_DEFAULT_STYLE_NAME);
        sprintf(style_name, "%s", style_value);
        WriteString(CONFIG_GLOBAL, CONFIG_STYLE_NAME, style_name);
        Style::SetStylePath(style_name);

        // Load global config
        swap_xo = ReadBool(CONFIG_GLOBAL, CONFIG_SWAP_XO, false);
        WriteBool(CONFIG_GLOBAL, CONFIG_SWAP_XO, swap_xo);

        bg_music_list_str = ReadString(CONFIG_GLOBAL, CONFIG_BACKGROUD_MUSIC, "ux0:/app/FTPCLI001/music.ogg");
        ParseMultiValueString(bg_music_list_str, bg_music_list, false);
        WriteString(CONFIG_GLOBAL, CONFIG_BACKGROUD_MUSIC, bg_music_list_str);

        enable_backgrou_music = ReadBool(CONFIG_GLOBAL, CONFIG_ENABLE_BACKGROUND_MUSIC, false);
        WriteBool(CONFIG_GLOBAL, CONFIG_ENABLE_BACKGROUND_MUSIC, enable_backgrou_music);
        
        sprintf(ftp_settings.server_ip, "%s", ReadString(CONFIG_GLOBAL, CONFIG_FTP_SERVER_IP, ""));
        WriteString(CONFIG_GLOBAL, CONFIG_FTP_SERVER_IP, ftp_settings.server_ip);

        ftp_settings.server_port = ReadInt(CONFIG_GLOBAL, CONFIG_FTP_SERVER_PORT, 21);
        WriteInt(CONFIG_GLOBAL, CONFIG_FTP_SERVER_PORT, ftp_settings.server_port);

        ftp_settings.pasv_mode = ReadBool(CONFIG_GLOBAL, CONFIG_FTP_TRANSFER_MODE, true);
        WriteBool(CONFIG_GLOBAL, CONFIG_FTP_TRANSFER_MODE, ftp_settings.pasv_mode);
        
        sprintf(ftp_settings.username, "%s", ReadString(CONFIG_GLOBAL, CONFIG_FTP_SERVER_USER, ""));
        WriteString(CONFIG_GLOBAL, CONFIG_FTP_SERVER_USER, ftp_settings.username);

        sprintf(ftp_settings.password, "%s", ReadString(CONFIG_GLOBAL, CONFIG_FTP_SERVER_PASSWORD, ""));
        WriteString(CONFIG_GLOBAL, CONFIG_FTP_SERVER_PASSWORD, ftp_settings.password);

        sprintf(local_directory, "%s", ReadString(CONFIG_GLOBAL, CONFIG_LOCAL_DIRECTORY, "ux0:"));
        WriteString(CONFIG_GLOBAL, CONFIG_LOCAL_DIRECTORY, local_directory);

        sprintf(remote_directory, "%s", ReadString(CONFIG_GLOBAL, CONFIG_REMOTE_DIRECTORY, "/"));
        WriteString(CONFIG_GLOBAL, CONFIG_REMOTE_DIRECTORY, remote_directory);

        WriteIniFile(CONFIG_INI_FILE);
        CloseIniFile();

        void *f = FS::OpenRead(FTP_CLIENT_VERSION_PATH);
        memset(app_ver, 0, sizeof(app_ver));
        FS::Read(f, app_ver, 3);
        FS::Close(f);
        float ver = atof(app_ver)/100;
        sprintf(app_ver, "%.2f", ver);
    }

    void SaveConfig()
    {
		OpenIniFile (CONFIG_INI_FILE);

        WriteString(CONFIG_GLOBAL, CONFIG_FTP_SERVER_IP, ftp_settings.server_ip);
        WriteInt(CONFIG_GLOBAL, CONFIG_FTP_SERVER_PORT, ftp_settings.server_port);
        WriteBool(CONFIG_GLOBAL, CONFIG_FTP_TRANSFER_MODE, ftp_settings.pasv_mode);
        WriteString(CONFIG_GLOBAL, CONFIG_FTP_SERVER_USER, ftp_settings.username);
        WriteString(CONFIG_GLOBAL, CONFIG_FTP_SERVER_PASSWORD, ftp_settings.password);

        WriteIniFile(CONFIG_INI_FILE);
        CloseIniFile();
    }

    void ParseMultiValueString(const char* prefix_list, std::vector<std::string> &prefixes, bool toLower)
    {
        std::string prefix = "";
        int length = strlen(prefix_list);
        for (int i=0; i<length; i++)
        {
            char c = prefix_list[i];
            if (c != ' ' && c != '\t' && c != ',')
            {
                if (toLower)
                {
                    prefix += std::tolower(c);
                }
                else
                {
                    prefix += c;
                }
            }
            
            if (c == ',' || i==length-1)
            {
                prefixes.push_back(prefix);
                prefix = "";
            }
        }
    }

    std::string GetMultiValueString(std::vector<std::string> &multi_values)
    {
        std::string vts = std::string("");
        if (multi_values.size() > 0)
        {
            for (int i=0; i<multi_values.size()-1; i++)
            {
                vts.append(multi_values[i]).append(",");
            }
            vts.append(multi_values[multi_values.size()-1]);
        }
        return vts;
    }

    void RemoveFromMultiValues(std::vector<std::string> &multi_values, std::string value)
    {
        auto itr = std::find(multi_values.begin(), multi_values.end(), value);
        if (itr != multi_values.end()) multi_values.erase(itr);
    }
}
