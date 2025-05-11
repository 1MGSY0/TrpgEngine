#pragma once

#include <string>
#include <vector>
#include <Windows.h>
#include <imgui.h>
#include <filesystem>

class EditorUI;

// --- File Dialogs ---
std::string openFileDialog(const char* filter = "All Files (*.*)\0*.*\0");
std::string saveFileDialog(const char* filter = "All Files (*.*)\0*.*\0");

// --- Drag & Drop ---
extern std::vector<std::string> s_pendingDroppedPaths;
void handleOSFileDrop(HDROP hDrop, EditorUI* editorUI);

// --- Theme ---
void applyCustomDarkTheme();

// --- Utility ---
bool endsWith(const std::string& str, const std::string& suffix);