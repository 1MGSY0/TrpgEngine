#include <imgui.h>
#include <Windows.h>
#include <commdlg.h>
#include <filesystem>
#include <string>

#include "UI/ImGuiUtils/ImGuiUtils.hpp"

namespace fs = std::filesystem;

// --- File Dialogs (with and without explicit filters) ---
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

// Default-filter overloads for convenience
std::string openFileDialog() {
    static const char* kDefaultFilter =
        "TRPG Project (*.trpgproj)\0*.trpgproj\0All Files (*.*)\0*.*\0";
    return openFileDialog(kDefaultFilter);
}

std::string saveFileDialog() {
    static const char* kDefaultFilter =
        "TRPG Project (*.trpgproj)\0*.trpgproj\0All Files (*.*)\0*.*\0";
    return saveFileDialog(kDefaultFilter);
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