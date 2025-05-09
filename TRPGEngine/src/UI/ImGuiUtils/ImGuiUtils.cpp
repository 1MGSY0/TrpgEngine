#include "UI/ImGUIUtils/ImGuiUtils.h"
#include <imgui.h>
#include <Windows.h>
#include <commdlg.h>
#include <filesystem>
namespace fs = std::filesystem;

#include <string>

static std::vector<std::string> s_droppedFiles;

std::string openFileDialog(const char* filter) {
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn = { sizeof(ofn) };
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter;
    ofn.lpstrDefExt = "trpgproj";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    return GetOpenFileNameA(&ofn) ? std::string(filename) : "";
}

std::string saveFileDialog(const char* filter) {
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn = { sizeof(ofn) };
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter;
    ofn.lpstrDefExt = "trpgproj";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (GetSaveFileNameA(&ofn)) {
        std::string result(filename);
        if (fs::path(result).extension() != ".trpgproj") {
            result += ".trpgproj";
        }
        return result;
    }

    return "";
}


void handleOSFileDrop(HWND hwnd) {
    s_droppedFiles.clear();

    HDROP hDrop = reinterpret_cast<HDROP>(GetClipboardData(CF_HDROP));
    if (!hDrop) return;

    UINT count = DragQueryFileA(hDrop, 0xFFFFFFFF, NULL, 0);
    char filePath[MAX_PATH];

    for (UINT i = 0; i < count; ++i) {
        if (DragQueryFileA(hDrop, i, filePath, MAX_PATH)) {
            s_droppedFiles.emplace_back(filePath);
        }
    }

    DragFinish(hDrop);
}

std::vector<std::string> getDroppedFiles() {
    return s_droppedFiles;
}

void applyCustomDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.FramePadding = ImVec2(10, 6);
    ImGui::StyleColorsDark();
}

