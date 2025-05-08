#pragma once
#include <string>

void applyCustomDarkTheme();

std::string openFileDialog(const char* filter = "TRPG Project Files (*.trpgproj)\0*.trpgproj\0");
std::string saveFileDialog(const char* filter = "TRPG Project Files (*.trpgproj)\0*.trpgproj\0");
