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
static ime_callback_t ime_cancelled = nullptr;

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
std::vector<FsEntry> remote_files;
std::set<FsEntry> multi_selected_local_files;
std::set<FsEntry> multi_selected_remote_files;
FsEntry *selected_local_file;
FsEntry *selected_remote_file;
ACTIONS selected_action;
CopyStruct copy_set;
char status_message[1024];
char local_file_to_select[256];
char remote_file_to_select[256];
char local_filter[32];
char remote_filter[32];
char editor_text[1024];
char activity_message[1024];
int selected_browser = 0;
int saved_selected_browser;
bool activity_inprogess = false;
bool stop_activity = false;

int confirm_state = -1;
char confirm_message[256];
ACTIONS action_to_take = ACTION_NONE;

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
        copy_set.type = COPY_TYPE_NONE;
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
                multi_selected_remote_files.clear();
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
            if (selected_browser & REMOTE_BROWSER && selected_remote_file != nullptr && strcmp(selected_remote_file->name, "..") != 0)
            {
                multi_selected_local_files.clear();
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
        if (ImGui::Selectable(ftp_settings.server_ip, false, ImGuiSelectableFlags_DontClosePopups, ImVec2(140, 0)))
        {
            ime_single_field = ftp_settings.server_ip;
            ResetImeCallbacks();
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
            ResetImeCallbacks();
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
            ResetImeCallbacks();
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
            ResetImeCallbacks();
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
            selected_action = ACTION_CONNECT_FTP;
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
            ResetImeCallbacks();
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
            ResetImeCallbacks();
            ime_after_update = AfterLocalFileChangesCallback;
            ime_callback = SingleValueImeCallback;
            Dialog::initImeDialog("Filter", local_filter, 31, SCE_IME_TYPE_DEFAULT, 0, 0);
            gui_mode = GUI_MODE_IME;
        }
        ImGui::PopID(); ImGui::SameLine();
        if (ImGui::SmallButton("Clear##local"))
        {
            selected_action = ACTION_CLEAR_LOCAL_FILTER;
        }
        ImGui::BeginChild("Local##ChildWindow", ImVec2(452,315));
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
                selected_action = ACTION_CHANGE_LOCAL_DIRECTORY;
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
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(it->display_size).x 
                   - ImGui::GetScrollX() - ImGui::GetStyle().ItemSpacing.x);
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
            ResetImeCallbacks();
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
            ResetImeCallbacks();
            ime_after_update = AfterRemoteFileChangesCallback;
            ime_callback = SingleValueImeCallback;
            Dialog::initImeDialog("Directory", remote_filter, 31, SCE_IME_TYPE_DEFAULT, 0, 0);
            gui_mode = GUI_MODE_IME;
        };
        ImGui::PopID(); ImGui::SameLine();
        if (ImGui::SmallButton("Clear##remote"))
        {
            selected_action = ACTION_CLEAR_REMOTE_FILTER;
        }
        ImGui::BeginChild(ImGui::GetID("Remote##ChildWindow"), ImVec2(452,315));
        ImGui::Separator();
        ImGui::Columns(2, "Remote##Columns", true);
        i=99999;
        for (std::vector<FsEntry>::iterator it=remote_files.begin(); it!=remote_files.end(); )
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
                selected_action = ACTION_CHANGE_REMOTE_DIRECTORY;
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
                selected_browser |= REMOTE_BROWSER;
            }
            ImGui::NextColumn();
            ImGui::SetColumnWidth(-1,90);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(it->display_size).x 
                   - ImGui::GetScrollX() - ImGui::GetStyle().ItemSpacing.x);
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

    void ShowActionsDialog()
    {
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;
        int flags;

        if (io.NavInputs[ImGuiNavInput_Input] == 1.0f)
        {
            if (!paused)
                saved_selected_browser = selected_browser;
            SetModalMode(true);
            ImGui::OpenPopup("Actions");
        }

        ImGui::SetNextWindowPos(ImVec2(340, 150));
        ImGui::SetNextWindowSizeConstraints(ImVec2(280,200), ImVec2(280,300), NULL, NULL);
        if (ImGui::BeginPopupModal("Actions", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            bool local_browser_selected = saved_selected_browser & LOCAL_BROWSER;
            bool remote_browser_selected = saved_selected_browser & REMOTE_BROWSER;
            if (copy_set.type == COPY_TYPE_LOCAL_ENTRY)
            {
                ImGui::TextColored(ImVec4(0.0f,1.0f,0.0f,1.0f), "%d Local file(s)/folder(s) copied.", copy_set.files.size());
                ImGui::Separator();
            } else if (copy_set.type == COPY_TYPE_REMOTE_ENTRY)
            {
                ImGui::TextColored(ImVec4(0.0f,1.0f,0.0f,1.0f), "%d Remote file(s)/folder(s) copied.", copy_set.files.size());
                ImGui::Separator();
            }

            flags = ImGuiSelectableFlags_Disabled;
            if (multi_selected_local_files.size() > 0 || multi_selected_remote_files.size() > 0)
                flags = ImGuiSelectableFlags_None;
            if (ImGui::Selectable("Copy##settings", false, flags | ImGuiSelectableFlags_DontClosePopups, ImVec2(270, 0)))
            {
                if (multi_selected_local_files.size() > 0)
                {
                    copy_set.files.clear();
                    copy_set.files.insert(multi_selected_local_files.begin(), multi_selected_local_files.end());
                    copy_set.type = COPY_TYPE_LOCAL_ENTRY;
                }
                else if (multi_selected_remote_files.size() > 0)
                {
                    copy_set.files.clear();
                    copy_set.files.insert(multi_selected_remote_files.begin(), multi_selected_remote_files.end());
                    copy_set.type = COPY_TYPE_REMOTE_ENTRY;
                }
                SetModalMode(false);
                selected_action = ACTION_COPY;
                ImGui::CloseCurrentPopup();
            }
            ImGui::Separator();

            char display_text[64];
            sprintf(display_text, "Paste##settings");
            flags = ImGuiSelectableFlags_Disabled;
            if (copy_set.type != COPY_TYPE_NONE && saved_selected_browser > 0)
            {
                flags = ImGuiSelectableFlags_None;
                // Disable pasting into the folder where files are being copied from
                if ((copy_set.type == COPY_TYPE_LOCAL_ENTRY && local_browser_selected && strcmp(copy_set.files.begin()->directory, local_directory) == 0) ||
                    (copy_set.type == COPY_TYPE_REMOTE_ENTRY && remote_browser_selected && strcmp(copy_set.files.begin()->directory, remote_directory) == 0))
                {
                    flags = ImGuiSelectableFlags_Disabled;
                }
                sprintf(display_text, "Paste into %s##settings", local_browser_selected ? "Local" : remote_browser_selected ? "Remote" : "" );
            }
            if (ImGui::Selectable(display_text, false, flags | ImGuiSelectableFlags_DontClosePopups, ImVec2(270, 0)))
            {
                selected_action = ACTION_PASTE;
            }
            ImGui::Separator();
            ImGui::Dummy(ImVec2(190, 10));
            flags = ImGuiSelectableFlags_Disabled;
            if (multi_selected_local_files.size() > 0 || multi_selected_remote_files.size() > 0)
                flags = ImGuiSelectableFlags_None;
            if (ImGui::Selectable("Delete##settings", false, flags | ImGuiSelectableFlags_DontClosePopups, ImVec2(270, 0)))
            {
                confirm_state = 0;
                sprintf(confirm_message, "Are you sure you want to delete this file(s)/folder(s) ?");
                action_to_take = ACTION_DELETE;
            }
            ImGui::Separator();
            if (ImGui::Selectable("Rename##settings", false, ImGuiSelectableFlags_Disabled | ImGuiSelectableFlags_DontClosePopups, ImVec2(270, 0)))
            {
                selected_action = ACTION_RENAME;
            }
            ImGui::Separator();

            sprintf(display_text, "New Folder##settings");
            flags = ImGuiSelectableFlags_Disabled;
            if (saved_selected_browser > 0)
            {
                flags = ImGuiSelectableFlags_None;
                sprintf(display_text, "New Folder in %s##settings", local_browser_selected ? "Local" : remote_browser_selected ? "Remote" : "" );
            }
            if (ImGui::Selectable(display_text, false, flags | ImGuiSelectableFlags_DontClosePopups, ImVec2(270, 0)))
            {
                if (local_browser_selected)
                    selected_action = ACTION_NEW_LOCAL_FOLDER;
                else if (remote_browser_selected)
                    selected_action = ACTION_NEW_REMOTE_FOLDER;
                SetModalMode(false);
                ImGui::CloseCurrentPopup();
            }
            ImGui::Dummy(ImVec2(190, 5));
            ImGui::Separator();
            if (ImGui::Selectable("Cancel##settings", false, ImGuiSelectableFlags_DontClosePopups, ImVec2(270, 0)))
            {
                SetModalMode(false);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsWindowAppearing())
            {
                SetNavFocusHere();
            }
            ImGui::EndPopup();
        }

        if (confirm_state == 0)
        {
            ImGui::OpenPopup("Confirm");
            ImGui::SetNextWindowPos(ImVec2(280, 200));
            ImGui::SetNextWindowSizeConstraints(ImVec2(420,100), ImVec2(430,200), NULL, NULL);
            if (ImGui::BeginPopupModal("Confirm", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text(confirm_message);
                ImGui::NewLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX()+150);
                if (ImGui::Button("Cancel"))
                {
                    confirm_state = 2;
                    ImGui::CloseCurrentPopup();
                };
                ImGui::SameLine();
                if (ImGui::Button("OK", ImVec2(60, 0)))
                {
                    confirm_state = 1;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
        else if (confirm_state > 0)
        {
            if (confirm_state == 2)
                selected_action = ACTION_NONE;
            else
                selected_action = action_to_take;
            confirm_state = -1;
        }

    }

    void ShowProgressDialog()
    {
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;

        SetModalMode(true);
        ImGui::OpenPopup("Progress");

        ImGui::SetNextWindowPos(ImVec2(230, 200));
        ImGui::SetNextWindowSizeConstraints(ImVec2(500,200), ImVec2(500,300), NULL, NULL);
        if (ImGui::BeginPopupModal("Progress", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 480);
            ImGui::Text("%s", activity_message);
            ImGui::Separator();
            if (ImGui::Button("Cancel"))
            {
                stop_activity = true;
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
            if (activity_inprogess)
            {
                ShowProgressDialog();
            }
            ShowActionsDialog();
        }
        ImGui::End();

        switch (selected_action)
        {
        case ACTION_CHANGE_LOCAL_DIRECTORY:
            Actions::HandleChangeLocalDirectory(selected_local_file);
            break;
        case ACTION_CHANGE_REMOTE_DIRECTORY:
            Actions::HandleChangeRemoteDirectory(selected_remote_file);
            break;
        case ACTION_REFRESH_LOCAL_FILES:
            Actions::HandleRefreshLocalFiles();
            break;
        case ACTION_REFRESH_REMOTE_FILES:
            Actions::HandleRefreshRemoteFiles();
            break;
        case ACTION_CLEAR_LOCAL_FILTER:
            Actions::HandleClearLocalFilter();
            break;
        case ACTION_CLEAR_REMOTE_FILTER:
            Actions::HandleClearRemoteFilter();
            break;
        case ACTION_NEW_LOCAL_FOLDER:
        case ACTION_NEW_REMOTE_FOLDER:
            if (gui_mode != GUI_MODE_IME)
            {
                sprintf(editor_text, "");
                ime_single_field = editor_text;
                ResetImeCallbacks();
                ime_after_update = AfterFolderNameCallback;
                ime_cancelled = CancelActionCallBack;
                ime_callback = SingleValueImeCallback;
                Dialog::initImeDialog("New Folder", editor_text, 128, SCE_IME_TYPE_DEFAULT, 0, 0);
                gui_mode = GUI_MODE_IME;
            }
            break;
        case ACTION_DELETE:
            if (multi_selected_local_files.size() > 0)
            {
                activity_inprogess = true;
                stop_activity = false;
                selected_action = ACTION_NONE;
                Actions::DeleteSelectedLocalFiles();
            }
            break;
        case ACTION_CONNECT_FTP:
            Actions::ConnectFTP();
            break;
        default:
            break;
        }
        Windows::EndSetupWindow();
    }

    void ResetImeCallbacks()
    {
        ime_callback = nullptr;
        ime_after_update = nullptr;
        ime_before_update = nullptr;
        ime_cancelled = nullptr;
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
            else if (ime_cancelled != nullptr)
            {
                ime_cancelled(ime_result);
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
        selected_action = ACTION_REFRESH_LOCAL_FILES;
    }

    void AfterRemoteFileChangesCallback(int ime_result)
    {
        std::string str = std::string(remote_directory);
        str = Util::Trim(str, " ");
        if (strcmp(str.c_str(), "/") != 0)
        {
            sprintf(remote_directory, "%s", Util::Rtrim(str, "/").c_str());
        }
        selected_action = ACTION_REFRESH_REMOTE_FILES;
    }

    void AfterFolderNameCallback(int ime_result)
    {
        if (selected_action == ACTION_NEW_LOCAL_FOLDER)
        {
            Actions::CreateNewLocalFolder(editor_text);
        }
        else if (selected_action == ACTION_NEW_REMOTE_FOLDER)
        {
            Actions::CreateNewRemoteFolder(editor_text);
        }
        selected_action = ACTION_NONE;
    }

    void CancelActionCallBack(int ime_result)
    {
        selected_action = ACTION_NONE;
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
