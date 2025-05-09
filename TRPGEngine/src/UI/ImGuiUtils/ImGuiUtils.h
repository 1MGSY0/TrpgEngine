#pragma once
#include <Windows.h>
#include <string>
#include <vector> 

std::string openFileDialog(const char* filter = "All Files\0*.*\0");
std::string saveFileDialog(const char* filter = "All Files\0*.*\0");
void applyCustomDarkTheme();

// NEW:
void handleOSFileDrop(HWND hwnd);
std::vector<std::string> getDroppedFiles();
