#pragma once
#include <string>

class ProjectManager {
public:
    static bool saveProject(const std::string& filePath);
    static bool loadProject(const std::string& filePath);
};
