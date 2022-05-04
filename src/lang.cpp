#include <vitasdk.h>
#include "string.h"
#include "stdio.h"
#include "lang.h"

char lang_identifiers[LANG_STRINGS_NUM][LANG_ID_SIZE] = {
	FOREACH_STR(GET_STRING)
};

// This is properly populated so that emulator won't crash if an user launches it without language INI files.
char lang_strings[LANG_STRINGS_NUM][LANG_STR_SIZE] = {
	"Connection Settings", // STR_GP_CONNECTION_SETTINGS
	"Local", // STR_GP_LOCAL
	"Remote", // STR_GP_REMOTE
	"Messages", // STR_GP_MESSAGES
	"Update Software", // STR_TT_UPDATE_SOFTWARE
	"Connect FTP", // STR_TT_CONNECT_FTP
	"Disconnect FTP", // STR_TT_DISCONNECT_FTP
	"Search",  // STR_TT_SEARCH
	"Refresh",  // STR_TT_REFRESH
	"Server", // STR_TT_SERVER
	"Username",  // STR_LB_USERNAME
	"Password",  // STR_LB_PASSWORD
	"Port", // STR_LB_PORT
	"Pasv", // STR_LB_PASV
	"Directory",  // STR_LB_DIRECTORY
	"Filter", // STR_LB_FILTER
	"Yes", // STR_LB_YES
	"No", // STR_LB_NO
	"Cancel", // STR_LB_CANCEL
	"Continue", // STR_LB_CONTINUE
	"Close", // STR_LB_CLOSE
	"Folder", // STR_LB_FOLDER
	"File", // STR_LB_FILE
	"Type", // STR_LB_TYPE
	"Name", // STR_LB_NAME
	"Size", // STR_LB_SIZE
	"Date", // STR_LB_DATE
	"New Folder", // STR_LB_NEW_FOLDER
	"Rename", // STR_LB_RENAME
	"Delete", // STR_LB_DELETE
	"Upload", // STR_LB_UPLOAD
	"Download", // STR_LB_DOWNLOAD
	"Select All", // STR_LB_SELECT_ALL
	"Clear All", // STR_LB_CLEAR_ALL
	"Uploading", // STR_LB_UPLOADING
	"Downloading", // STR_LB_DOWNLOADING
	"Overwrite", // STR_LB_OVERWRITE
	"Don't Overwrite", // STR_LB_DONT_OVERWRITE
	"Ask for Confirmation", // STR_LB_ASK_FOR_CONFIRM
	"Don't Ask for Confirmation", // STR_LB_DONT_ASK_CONFIRM
	"Always use this option and don't ask again", // STR_LB_ALLWAYS_USE_OPTION
	"Actions", // STR_DLG_LB_ACTIONS
	"Confirm", // STR_DLG_LB_CONFIRM
	"Overwrite Options", // STR_DLG_LB_OVERWRITE_OPTIONS
	"Properties", // STR_DLG_LB_PROPERTIES
	"Progress", // STR_DLG_LB_PROGRESS
	"Updates", // STR_DLG_LB_UPDATES
	"Are you sure you want to delete this file(s)/folder(s)?", // STR_DEL_CONFIRM_MSG
	"Canceling. Waiting for last action to complete", // STR_CANCEL_ACTION_MSG
	"Failed to upload file", // STR_FAIL_UPLOAD_MSG
	"Failed to downloadload file", // STR_FAIL_DOWNLOAD_MSG
	"Failed to read contents of directory or folder does not exists.", // STR_FAIL_READ_LOCAL_DIR_MSG
	"426 Connection closed", // STR_CONNECTION_CLOSE_ERR_MSG
	"426 Remote Server has terminated the connection", // STR_REMOTE_TERM_CONN_MSG
	"300 Failed Login. Please check your username or password.", // STR_FAIL_LOGIN_MSG
	"426 Failed. Connection timeout." // STR_FAIL_TIMEOUT_MSG
};

bool needs_extended_font = false;

namespace Lang
{
	void SetTranslation(int idx)
	{
		char langFile[LANG_STR_SIZE * 2];
		char identifier[LANG_ID_SIZE], buffer[LANG_STR_SIZE];
		
		switch (idx)
		{
			case SCE_SYSTEM_PARAM_LANG_ITALIAN:
				sprintf(langFile, "ux0:app/FTPCLI001/lang/Italian.ini");
				break;
			case SCE_SYSTEM_PARAM_LANG_SPANISH:
				sprintf(langFile, "ux0:app/FTPCLI001/lang/Spanish.ini");
				break;
			case SCE_SYSTEM_PARAM_LANG_GERMAN:
				sprintf(langFile, "ux0:app/FTPCLI001/lang/German.ini");
				break;
			case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_PT: // FIXME: Temporarily using Brazilian one
			case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_BR:
				sprintf(langFile, "ux0:app/FTPCLI001/lang/Portuguese_BR.ini");
				break;
			case SCE_SYSTEM_PARAM_LANG_RUSSIAN:
				sprintf(langFile, "ux0:app/FTPCLI001/lang/Russian.ini");
				break;
			case SCE_SYSTEM_PARAM_LANG_JAPANESE:
				sprintf(langFile, "ux0:app/FTPCLI001/lang/Japanese.ini");
				needs_extended_font = true;
				break;
			case SCE_SYSTEM_PARAM_LANG_UKRAINIAN:
				sprintf(langFile, "ux0:app/FTPCLI001/lang/Ukrainian.ini");
				break;
			default:
				sprintf(langFile, "ux0:app/FTPCLI001/lang/English.ini");
				break;
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
	}
}