
#include <imgui.h>
#include <Windows.h>
#include <commdlg.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <shellapi.h>

#include "UI/EditorUI.h"
#include "ImGuiUtils.h"

namespace fs = std::filesystem;
static std::vector<std::string> s_droppedFiles;

// --- File Dialogs ---
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
        if (fs::path(result).extension() != ".trpgproj")
            result += ".trpgproj";
        return result;
    }
    return "";
}

void handleOSFileDrop(HDROP hDrop, EditorUI* editor) {
    if (!hDrop || !editor) return;

    UINT fileCount = DragQueryFileA(hDrop, 0xFFFFFFFF, nullptr, 0);
    if (fileCount == 0) {
        editor->setStatusMessage("Drop failed: No files detected.");
        DragFinish(hDrop);
        return;
    }

    s_pendingDroppedPaths.clear();
    for (UINT i = 0; i < fileCount; ++i) {
        char filePath[MAX_PATH];
        if (DragQueryFileA(hDrop, i, filePath, MAX_PATH)) {
            s_pendingDroppedPaths.emplace_back(filePath);
        }
    }

    DragFinish(hDrop);
}

// --- Theme styling ---
void applyCustomDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.FramePadding = ImVec2(10, 6);
    ImGui::StyleColorsDark();
}

// --- Utility ---
bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}