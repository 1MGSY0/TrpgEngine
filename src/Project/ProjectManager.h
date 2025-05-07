#pragma once
#include <string>

class ProjectManager {
public:
    static bool save();
    static bool saveProjectToFile(const std::string& filePath);
    static bool loadProject(const std::string& filePath);

    static void setCurrentProjectPath(const std::string& filePath);
    static std::string getCurrentProjectPath();

    static void setTempLoadPath(const std::string& filePath);
    static std::string getTempLoadPath();

private:
    static std::string s_currentProjectPath;  // FULL path to .trpgproj
    static std::string s_tempLoadPath;
};
