#include <imgui_vita2d/imgui_vita.h>
#include <stdio.h>
#include <algorithm> 
#include "windows.h"
#include "textures.h"
#include "fs.h"
#include "style.h"
#include "config.h"
#include "ime_dialog.h"
#include "gui.h"
#include "net.h"
#include "ftpclient.h"
#include "updater.h"
#include "actions.h"

#include "debugnet.h"

extern "C" {
	#include "inifile.h"
}

static SceCtrlData pad_prev;
bool paused = false;
int view_mode;
static float scroll_direction = 0.0f;
static char cb_style_name[64];
static char cb_startup_category[10];
static char cb_category_name[16] = "all";
static std::vector<std::string> styles;
static ime_callback_t ime_callback = nullptr;
static ime_callback_t ime_after_update = nullptr;
static ime_callback_t ime_before_update = nullptr;

static std::vector<std::string> *ime_multi_field;
static char* ime_single_field;

bool handle_updates = false;
float previous_right = 0.0f;
float previous_left = 0.0f;
FtpClient *ftpclient;
int64_t bytes_transfered;
int64_t bytes_to_download;
std::vector<FsEntry> local_files;
std::vector<FtpDirEntry> remote_files;
FsEntry *selected_local_file;
FtpDirEntry *selected_remote_file;
ACTIONS selected_action;
char status_message[1024];
char local_file_to_select[256];
char remote_file_to_select[256];

namespace Windows {

    static int FtpCallback(int64_t xfered, void* arg)
    {
        bytes_transfered = xfered;
        return 1;
    }

    void Init()
    {
        ftpclient = new FtpClient();
        ftpclient->SetConnmode(ftp_settings.pasv_mode ? FtpClient::pasv : FtpClient::port);
        ftpclient->SetCallbackBytes(1);
        ftpclient->SetCallbackXferFunction(FtpCallback);

        local_files.clear();
        local_files = FS::ListDir(local_directory);
        FS::Sort(local_files);

        sprintf(local_file_to_select, "..");
        sprintf(remote_file_to_select, "..");
        sprintf(status_message, "");
    }

    void HandleLauncherWindowInput()
    {
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        SceCtrlData pad;

        sceCtrlReadBufferPositiveExt2(0, &pad, 1);

        pad_prev = pad;
        previous_right = io.NavInputs[ImGuiNavInput_DpadRight];
        previous_left = io.NavInputs[ImGuiNavInput_DpadLeft];
    }

    void SetModalMode(bool modal)
    {
        paused = modal;
    }

    void ConnectionPanel()
    {
        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;

        BeginGroupPanel("Connection Settings", ImVec2(945, 100));
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+3);
        char id[256];
        std::string hidden_password = std::string("********");

        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Server:"); ImGui::SameLine();
        ImGui::Selectable(ftp_settings.server_ip, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(140, 0)); ImGui::SameLine();
        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Username:"); ImGui::SameLine();
        sprintf(id, "%s##username", ftp_settings.username);
        ImGui::Selectable(id, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(120, 0)); ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Password:"); ImGui::SameLine();
        sprintf(id, "%s##password", hidden_password.c_str());
        ImGui::Selectable(id, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(100, 0)); ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Port:"); ImGui::SameLine();
        sprintf(id, "%d##ServerPort", ftp_settings.server_port);
        ImGui::Selectable(id, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(30, 0)); ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Pasv:"); ImGui::SameLine();
        ImGui::Checkbox("##PasvMode", &ftp_settings.pasv_mode); ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
        if (ImGui::Button("Connect"))
        {
            selected_action = CONNECT_FTP;
        }
        ImGui::Dummy(ImVec2(1,2));
        EndGroupPanel();
    }

    void BrowserPanel()
    {
        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;
        
        BeginGroupPanel("Local", ImVec2(452, 400));
        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Directory:"); ImGui::SameLine();
        ImGui::Selectable(local_directory, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(360, 0));
        ImGui::BeginChild(ImGui::GetID("Local##ChildWindow"), ImVec2(452,340));
        ImGui::Separator();
        ImGui::Columns(2, "Local##Columns", true);
        int i = 0;
        for (std::vector<FsEntry>::iterator it=local_files.begin(); it!=local_files.end(); )
        {
            ImGui::SetColumnWidth(-1,343);
            ImGui::PushID(i);
            if (ImGui::Selectable(it->name, false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(452, 0)))
            {
                selected_local_file = &*it;
                selected_action = CHANGE_LOCAL_DIRECTORY;
            }
            ImGui::PopID();
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
            {
                if (strcmp(local_file_to_select, it->name)==0)
                {
                    SetNavFocusHere();
                    sprintf(local_file_to_select, "");
                }
            }
            ImGui::NextColumn();
            ImGui::SetColumnWidth(-1,90);
            ImGui::Text(it->display_size);
            ImGui::NextColumn();               
            ImGui::Separator();
            ++it;
            i++;
        }
        ImGui::Columns(1);
        ImGui::EndChild();
        EndGroupPanel();
        ImGui::SameLine();

        BeginGroupPanel("Remote", ImVec2(452, 400));
        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Directory:"); ImGui::SameLine();
        ImGui::Selectable(remote_directory, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(360, 0));
        ImGui::BeginChild(ImGui::GetID("Remote##ChildWindow"), ImVec2(452,340));
        ImGui::Separator();
        ImGui::Columns(2, "Remote##Columns", true);
        i=99999;
        for (std::vector<FtpDirEntry>::iterator it=remote_files.begin(); it!=remote_files.end(); )
        {
            ImGui::SetColumnWidth(-1,343);
            ImGui::PushID(i);
            if (ImGui::Selectable(it->name, false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(452, 0)))
            {
                selected_remote_file = &*it;
                selected_action = CHANGE_REMOTE_DIRECTORY;
            }
            ImGui::PopID();
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
            {
                if (strcmp(remote_file_to_select, it->name)==0)
                {
                    SetNavFocusHere();
                    sprintf(remote_file_to_select, "");
                }
            }
            ImGui::NextColumn();
            ImGui::SetColumnWidth(-1,90);
            ImGui::Text(it->display_size);
            ImGui::NextColumn();               
            ImGui::Separator();
            ++it;
            i++;
        }
        ImGui::Columns(1);
        ImGui::EndChild();
        EndGroupPanel();
    }

    void StatusPanel()
    {
        BeginGroupPanel("Status", ImVec2(945, 100));
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::Dummy(ImVec2(925,50));
        ImGui::SetCursorPos(pos);
        ImGui::PushTextWrapPos(925);
        ImGui::Text(status_message);
        ImGui::PopTextWrapPos();
        EndGroupPanel();
    }

    void ShowSettingsActionsDialog()
    {

    }

    void MainWindow()
    {
        Windows::SetupWindow();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::SetMouseCursor(ImGuiMouseCursor_None);

        if (ImGui::Begin("Ftp Client", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar))
        {
            ConnectionPanel();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY()+3);
            BrowserPanel();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY()+3);
            StatusPanel();
            ShowSettingsActionsDialog();
        }
        ImGui::End();

        switch (selected_action)
        {
        case CHANGE_LOCAL_DIRECTORY:
            Actions::HandleChangeLocalDirectory(selected_local_file);
            break;
        case CHANGE_REMOTE_DIRECTORY:
            Actions::HandleChangeRemoteDirectory(selected_remote_file);
            break;
        case CONNECT_FTP:
            Actions::ConnectFTP();
            break;
        default:
            break;
        }
        Windows::EndSetupWindow();
    }

    void HandleImeInput()
    {
        int ime_result = Dialog::updateImeDialog();

        if (ime_result == IME_DIALOG_RESULT_FINISHED || ime_result == IME_DIALOG_RESULT_CANCELED)
        {
            if (ime_result == IME_DIALOG_RESULT_FINISHED)
            {
                if (ime_before_update != nullptr)
                {
                    ime_before_update(ime_result);
                }

                if (ime_callback != nullptr)
                {
                    ime_callback(ime_result);
                }

                if (ime_after_update != nullptr)
                {
                    ime_after_update(ime_result);
                }
            }

            gui_mode = GUI_MODE_BROWSER;
        }
    }

    void SingleValueImeCallback(int ime_result)
    {
        if (ime_result == IME_DIALOG_RESULT_FINISHED)
        {
            char *new_value = (char *)Dialog::getImeDialogInputTextUTF8();
            sprintf(ime_single_field, "%s", new_value);
        }
    }

    void MultiValueImeCallback(int ime_result)
    {
        if (ime_result == IME_DIALOG_RESULT_FINISHED)
        {
            char *new_value = (char *)Dialog::getImeDialogInputTextUTF8();
            char *initial_value = (char *)Dialog::getImeDialogInitialText();
            if (strlen(initial_value) == 0)
            {
                ime_multi_field->push_back(new_value);
            }
            else
            {
                for (int i=0; i < ime_multi_field->size(); i++)
                {
                    if (strcmp((*ime_multi_field)[i].c_str(), initial_value)==0)
                    {
                        (*ime_multi_field)[i] = new_value;
                    }
                }
            }
            
        }
    }

    void NullAfterValueChangeCallback(int ime_result) {}

    void HandleUpdates()
    {
        SetModalMode(true);

        ImGui::OpenPopup("Updates");
        ImGui::SetNextWindowPos(ImVec2(300, 200));
        ImGui::SetNextWindowSize(ImVec2(400,90));
        if (ImGui::BeginPopupModal("Updates", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(400,140));
            ImGui::Text("%s", updater_message);
            ImGui::EndPopup();
        }
    }

}
