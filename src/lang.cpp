#include <vitasdk.h>
#include "string.h"
#include "stdio.h"
#include "config.h"
#include "util.h"
#include "lang.h"

char lang_identifiers[LANG_STRINGS_NUM][LANG_ID_SIZE] = {
	FOREACH_STR(GET_STRING)
};

// This is properly populated so that emulator won't crash if an user launches it without language INI files.
char lang_strings[LANG_STRINGS_NUM][LANG_STR_SIZE] = {
	"Connection Settings", // STR_CONNECTION_SETTINGS
	"Site", // STR_SITE
	"Local", // STR_LOCAL
	"Remote", // STR_REMOTE
	"Messages", // STR_MESSAGES
	"Update Software", // STR_UPDATE_SOFTWARE
	"Connect FTP", // STR_CONNECT_FTP
	"Disconnect FTP", // STR_DISCONNECT_FTP
	"Search",  // STR_SEARCH
	"Refresh",  // STR_REFRESH
	"Server", // STR_SERVER
	"Username",  // STR_USERNAME
	"Password",  // STR_PASSWORD
	"Port", // STR_PORT
	"Pasv", // STR_PASV
	"Directory",  // STR_DIRECTORY
	"Filter", // STR_FILTER
	"Yes", // STR_YES
	"No", // STR_NO
	"Cancel", // STR_CANCEL
	"Continue", // STR_CONTINUE
	"Close", // STR_CLOSE
	"Folder", // STR_FOLDER
	"File", // STR_FILE
	"Type", // STR_TYPE
	"Name", // STR_NAME
	"Size", // STR_SIZE
	"Date", // STR_DATE
	"New Folder", // STR_NEW_FOLDER
	"Rename", // STR_RENAME
	"Delete", // STR_DELETE
	"Upload", // STR_UPLOAD
	"Download", // STR_DOWNLOAD
	"Select All", // STR_SELECT_ALL
	"Clear All", // STR_CLEAR_ALL
	"Uploading", // STR_UPLOADING
	"Downloading", // STR_DOWNLOADING
	"Overwrite", // STR_OVERWRITE
	"Don't Overwrite", // STR_DONT_OVERWRITE
	"Ask for Confirmation", // STR_ASK_FOR_CONFIRM
	"Don't Ask for Confirmation", // STR_DONT_ASK_CONFIRM
	"Always use this option and don't ask again", // STR_ALLWAYS_USE_OPTION
	"Actions", // STR_ACTIONS
	"Confirm", // STR_CONFIRM
	"Overwrite Options", // STR_OVERWRITE_OPTIONS
	"Properties", // STR_PROPERTIES
	"Progress", // STR_PROGRESS
	"Updates", // STR_UPDATES
	"Are you sure you want to delete this file(s)/folder(s)?", // STR_DEL_CONFIRM_MSG
	"Canceling. Waiting for last action to complete", // STR_CANCEL_ACTION_MSG
	"Failed to upload file", // STR_FAIL_UPLOAD_MSG
	"Failed to download file", // STR_FAIL_DOWNLOAD_MSG
	"Failed to read contents of directory or folder does not exist.", // STR_FAIL_READ_LOCAL_DIR_MSG
	"426 Connection closed.", // STR_CONNECTION_CLOSE_ERR_MSG
	"426 Remote Server has terminated the connection.", // STR_REMOTE_TERM_CONN_MSG
	"300 Failed Login. Please check your username or password.", // STR_FAIL_LOGIN_MSG
	"426 Failed. Connection timeout.", // STR_FAIL_TIMEOUT_MSG
	"Failed to delete directory", // STR_FAIL_DEL_DIR_MSG
	"Deleting", // STR_DELETING
	"Failed to delete file", // STR_FAIL_DEL_FILE_MSG
	"Deleted" // STR_DELETED
};

bool needs_extended_font = false;

namespace Lang
{
	void SetTranslation(int idx)
	{
		char langFile[LANG_STR_SIZE * 2];
		char identifier[LANG_ID_SIZE], buffer[LANG_STR_SIZE];
		
		std::string lang = std::string(language);
		lang = Util::Trim(lang, " ");
		if (lang.size() > 0)
		{
			sprintf(langFile, "ux0:app/FTPCLI001/lang/%s.ini", lang.c_str());
		}
		else
		{
			switch (idx)
			{
				case SCE_SYSTEM_PARAM_LANG_ITALIAN:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/Italiano.ini");
					break;
				case SCE_SYSTEM_PARAM_LANG_SPANISH:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/Spanish.ini");
					break;
				case SCE_SYSTEM_PARAM_LANG_GERMAN:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/German.ini");
					break;
				case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_PT:
				case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_BR:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/Portuguese_BR.ini");
					break;
				case SCE_SYSTEM_PARAM_LANG_RUSSIAN:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/Russian.ini");
					break;
				case SCE_SYSTEM_PARAM_LANG_DUTCH:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/Dutch.ini");
					break;
				case SCE_SYSTEM_PARAM_LANG_FRENCH:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/French.ini");
					break;
				case SCE_SYSTEM_PARAM_LANG_POLISH:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/Polish.ini");
					break;
				case SCE_SYSTEM_PARAM_LANG_JAPANESE:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/Japanese.ini");
					break;
				case SCE_SYSTEM_PARAM_LANG_KOREAN:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/Korean.ini");
					break;
				case SCE_SYSTEM_PARAM_LANG_CHINESE_S:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/Chinese_Simplified.ini");
					break;
				case SCE_SYSTEM_PARAM_LANG_CHINESE_T:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/Chinese_Traditional.ini");
					break;
				default:
					sprintf(langFile, "ux0:app/FTPCLI001/lang/English.ini");
					break;
			}
		}
		
		FILE *config = fopen(langFile, "r");
		if (config)
		{
			while (EOF != fscanf(config, "%[^=]=%[^\n]\n", identifier, buffer))
			{
				for (int i = 0; i < LANG_STRINGS_NUM; i++) {
					if (strcmp(lang_identifiers[i], identifier) == 0) {
						char *newline = nullptr, *p = buffer;
						while (newline = strstr(p, "\\n")) {
							newline[0] = '\n';
							int len = strlen(&newline[2]);
							memmove(&newline[1], &newline[2], len);
							newline[len + 1] = 0;
							p++;
						}
						strcpy(lang_strings[i], buffer);
					}
				}
			}
			fclose(config);
		}

        char buf[12];
        int num;
        sscanf(last_site, "%[^ ] %d", buf, &num);
        sprintf(display_site, "%s %d", lang_strings[STR_SITE], num);
	}
}