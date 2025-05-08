#include "ImGUIUtils/ImGuiUtils.h"
#include <imgui.h>
#include <Windows.h>
#include <commdlg.h>
#include <filesystem>
namespace fs = std::filesystem;

#include <string>

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

void applyCustomDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.FramePadding = ImVec2(10, 6);
    ImGui::StyleColorsDark();
}
