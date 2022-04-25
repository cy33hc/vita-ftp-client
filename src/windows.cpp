#include <imgui_vita2d/imgui_vita.h>
#include <stdio.h>
#include <algorithm>
#include <set>
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
#include "util.h"

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
static char txt_server_port[6];

bool handle_updates = false;
float previous_right = 0.0f;
float previous_left = 0.0f;
FtpClient *ftpclient;
int64_t bytes_transfered;
int64_t bytes_to_download;
std::vector<FsEntry> local_files;
std::vector<FtpDirEntry> remote_files;
std::set<FsEntry> multi_selected_local_files;
std::set<FtpDirEntry> multi_selected_remote_files;
FsEntry *selected_local_file;
FtpDirEntry *selected_remote_file;
ACTIONS selected_action;
char status_message[1024];
char local_file_to_select[256];
char remote_file_to_select[256];
char local_filter[32];
char remote_filter[32];
int selected_browser = 0;
int saved_selected_browser;

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

        sprintf(local_file_to_select, "..");
        sprintf(remote_file_to_select, "..");
        sprintf(status_message, "");
        sprintf(local_filter, "");
        sprintf(remote_filter, "");
        sprintf(txt_server_port, "%d", ftp_settings.server_port);

        Actions::RefreshLocalFiles();
    }

    void HandleWindowInput()
    {
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        SceCtrlData pad;

        sceCtrlReadBufferPositiveExt2(0, &pad, 1);

        if ((pad_prev.buttons & SCE_CTRL_SQUARE) && !(pad.buttons & SCE_CTRL_SQUARE) && !paused)
        {
            if ( selected_browser & LOCAL_BROWSER && selected_local_file != nullptr && strcmp(selected_local_file->name, "..") != 0)
            {
                auto search_item = multi_selected_local_files.find(*selected_local_file);
                if (search_item != multi_selected_local_files.end())
                {
                    multi_selected_local_files.erase(search_item);
                }
                else
                {
                    multi_selected_local_files.insert(*selected_local_file);
                }
            }
            if (selected_browser & REMOTE_BROWER && selected_remote_file != nullptr && strcmp(selected_remote_file->name, "..") != 0)
            {
                auto search_item = multi_selected_remote_files.find(*selected_remote_file);
                if (search_item != multi_selected_remote_files.end())
                {
                    multi_selected_remote_files.erase(search_item);
                }
                else
                {
                    multi_selected_remote_files.insert(*selected_remote_file);
                }
            }
        }

        pad_prev = pad;
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
        if (ImGui::Selectable(ftp_settings.server_ip, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(140, 0)))
        {
            ime_single_field = ftp_settings.server_ip;
            ime_before_update = nullptr;
            ime_after_update = nullptr;
            ime_callback = SingleValueImeCallback;
            Dialog::initImeDialog("Server IP", ftp_settings.server_ip, 15, SCE_IME_TYPE_DEFAULT, 0, 0);
            gui_mode = GUI_MODE_IME;
        }
        ImGui::SameLine();

        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Username:"); ImGui::SameLine();
        sprintf(id, "%s##username", ftp_settings.username);
        if (ImGui::Selectable(id, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(120, 0)))
        {
            ime_single_field = ftp_settings.username;
            ime_before_update = nullptr;
            ime_after_update = nullptr;
            ime_callback = SingleValueImeCallback;
            Dialog::initImeDialog("Username", ftp_settings.username, 32, SCE_IME_TYPE_DEFAULT, 0, 0);
            gui_mode = GUI_MODE_IME;
        }
        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Password:"); ImGui::SameLine();
        sprintf(id, "%s##password", hidden_password.c_str());
        if (ImGui::Selectable(id, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(100, 0)))
        {
            ime_single_field = ftp_settings.password;
            ime_before_update = nullptr;
            ime_after_update = nullptr;
            ime_callback = SingleValueImeCallback;
            Dialog::initImeDialog("Password", ftp_settings.password, 24, SCE_IME_TYPE_DEFAULT, 0, 0);
            gui_mode = GUI_MODE_IME;
        }
        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Port:"); ImGui::SameLine();
        sprintf(id, "%s##ServerPort", txt_server_port);
        if (ImGui::Selectable(id, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(30, 0)))
        {
            ime_single_field = txt_server_port;
            ime_before_update = nullptr;
            ime_after_update = nullptr;
            ime_callback = SingleValueImeCallback;
            Dialog::initImeDialog("Server Port", txt_server_port, 5, SCE_IME_TYPE_NUMBER, 0, 0);
            gui_mode = GUI_MODE_IME;
        }
        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Pasv:"); ImGui::SameLine();
        ImGui::Checkbox("##PasvMode", &ftp_settings.pasv_mode); ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
        if (ImGui::Button("Connect"))
        {
            ftp_settings.server_port = atoi(txt_server_port);
            selected_action = CONNECT_FTP;
        }
        ImGui::Dummy(ImVec2(1,2));
        EndGroupPanel();
    }

    void BrowserPanel()
    {
        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;
        selected_browser = 0;

        BeginGroupPanel("Local", ImVec2(452, 400));
        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Directory:"); ImGui::SameLine();
        if (ImGui::Selectable(local_directory, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(360, 0)))
        {
            ime_single_field = local_directory;
            ime_before_update = nullptr;
            ime_after_update = AfterLocalFileChangesCallback;
            ime_callback = SingleValueImeCallback;
            Dialog::initImeDialog("Directory", local_directory, 256, SCE_IME_TYPE_DEFAULT, 0, 0);
            gui_mode = GUI_MODE_IME;
        }
        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Filter:"); ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+30);
        ImGui::PushID("local_filter##remote");
        if (ImGui::Selectable(local_filter, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(310, 0)))
        {
            ime_single_field = local_filter;
            ime_before_update = nullptr;
            ime_after_update = AfterLocalFileChangesCallback;
            ime_callback = SingleValueImeCallback;
            Dialog::initImeDialog("Filter", local_filter, 31, SCE_IME_TYPE_DEFAULT, 0, 0);
            gui_mode = GUI_MODE_IME;
        }
        ImGui::PopID(); ImGui::SameLine();
        if (ImGui::SmallButton("Clear##local"))
        {
            selected_action = CLEAR_LOCAL_FILTER;
        }
        ImGui::BeginChild(ImGui::GetID("Local##ChildWindow"), ImVec2(452,315));
        ImGui::Separator();
        ImGui::Columns(2, "Local##Columns", true);
        int i = 0;
        for (std::vector<FsEntry>::iterator it=local_files.begin(); it!=local_files.end(); )
        {
            ImGui::SetColumnWidth(-1,343);
            ImGui::PushID(i);
            auto search_item = multi_selected_local_files.find(*it);
            if (search_item != multi_selected_local_files.end())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
            }
            if (ImGui::Selectable(it->name, false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(452, 0)))
            {
                selected_local_file = &*it;
                selected_action = CHANGE_LOCAL_DIRECTORY;
            }
            ImGui::PopID();
            if (ImGui::IsItemFocused())
            {
                selected_local_file = &*it;
            }
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
            {
                if (strcmp(local_file_to_select, it->name)==0)
                {
                    SetNavFocusHere();
                    ImGui::SetScrollHereY(0.0f);
                    sprintf(local_file_to_select, "");
                }
                selected_browser |= LOCAL_BROWSER;
            }
            ImGui::NextColumn();
            ImGui::SetColumnWidth(-1,90);
            ImGui::Text(it->display_size);
            if (search_item != multi_selected_local_files.end())
            {
                ImGui::PopStyleColor();
            }
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
        if (ImGui::Selectable(remote_directory, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(360, 0)))
        {
            ime_single_field = remote_directory;
            ime_before_update = nullptr;
            ime_after_update = AfterRemoteFileChangesCallback;
            ime_callback = SingleValueImeCallback;
            Dialog::initImeDialog("Directory", remote_directory, 256, SCE_IME_TYPE_DEFAULT, 0, 0);
            gui_mode = GUI_MODE_IME;
        }
        ImGui::TextColored(colors[ImGuiCol_ButtonHovered], "Filter:"); ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+30);
        ImGui::PushID("remote_filter##remote");
        if (ImGui::Selectable(remote_filter, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(310, 0)))
        {
            ime_single_field = remote_filter;
            ime_before_update = nullptr;
            ime_after_update = AfterRemoteFileChangesCallback;
            ime_callback = SingleValueImeCallback;
            Dialog::initImeDialog("Directory", remote_filter, 31, SCE_IME_TYPE_DEFAULT, 0, 0);
            gui_mode = GUI_MODE_IME;
        };
        ImGui::PopID(); ImGui::SameLine();
        if (ImGui::SmallButton("Clear##remote"))
        {
            selected_action = CLEAR_REMOTE_FILTER;
        }
        ImGui::BeginChild(ImGui::GetID("Remote##ChildWindow"), ImVec2(452,315));
        ImGui::Separator();
        ImGui::Columns(2, "Remote##Columns", true);
        i=99999;
        for (std::vector<FtpDirEntry>::iterator it=remote_files.begin(); it!=remote_files.end(); )
        {
            ImGui::SetColumnWidth(-1,343);
            auto search_item = multi_selected_remote_files.find(*it);
            if (search_item != multi_selected_remote_files.end())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
            }
            ImGui::PushID(i);
            if (ImGui::Selectable(it->name, false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(452, 0)))
            {
                selected_remote_file = &*it;
                selected_action = CHANGE_REMOTE_DIRECTORY;
            }
            ImGui::PopID();
            if (ImGui::IsItemFocused())
            {
                selected_remote_file = &*it;
            }
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
            {
                if (strcmp(remote_file_to_select, it->name)==0)
                {
                    SetNavFocusHere();
                    ImGui::SetScrollHereY(0.0f);
                    sprintf(remote_file_to_select, "");
                }
                selected_browser |= REMOTE_BROWER;
            }
            ImGui::NextColumn();
            ImGui::SetColumnWidth(-1,90);
            ImGui::Text(it->display_size);
            if (search_item != multi_selected_remote_files.end())
            {
                ImGui::PopStyleColor();
            }
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
        BeginGroupPanel("Messages", ImVec2(945, 100));
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
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        if (io.NavInputs[ImGuiNavInput_Input] == 1.0f)
        {
            if (!paused)
                saved_selected_browser = selected_browser;
            debugNetPrintf(DEBUG, "saved_selected_browser %d\n", saved_selected_browser);
            SetModalMode(true);
            ImGui::OpenPopup("Settings and Actions");
        }

        ImGui::SetNextWindowPos(ImVec2(250, 200));
        ImGui::SetNextWindowSizeConstraints(ImVec2(500,130), ImVec2(500,475), NULL, NULL);
        if (ImGui::BeginPopupModal("Settings and Actions", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            char label[32];
            sprintf(label, "Copy %d##settings", saved_selected_browser);
            if (ImGui::Selectable(label, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(452, 0)))
            {
                selected_action = COPY_LOCAL;
            }
            if (ImGui::Button("OK"))
            {
                SetModalMode(false);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

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
        case REFRESH_LOCAL_FILES:
            Actions::HandleRefreshLocalFiles();
            break;
        case REFRESH_REMOTE_FILES:
            Actions::HandleRefreshRemoteFiles();
            break;
        case CLEAR_LOCAL_FILTER:
            Actions::HandleClearLocalFilter();
            break;
        case CLEAR_REMOTE_FILTER:
            Actions::HandleClearRemoteFilter();
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

    void AfterLocalFileChangesCallback(int ime_result)
    {
        std::string str = std::string(local_directory);
        sprintf(local_directory, "%s", Util::Rtrim(Util::Trim(str, " "), "/").c_str());
        selected_action = REFRESH_LOCAL_FILES;
    }

    void AfterRemoteFileChangesCallback(int ime_result)
    {
        std::string str = std::string(remote_directory);
        str = Util::Trim(str, " ");
        if (strcmp(str.c_str(), "/") != 0)
        {
            sprintf(remote_directory, "%s", Util::Rtrim(str, "/").c_str());
        }
        selected_action = REFRESH_REMOTE_FILES;
    }

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
