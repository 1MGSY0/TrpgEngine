#pragma once
#include <string>

class ProjectManager {
public:
    static std::string getCurrentProjectPath();
    static void setCurrentProjectPath(const std::string& path);

    static std::string getTempLoadPath();
    static void setTempLoadPath(const std::string& path);

    static bool loadProject(const std::string& path);
    static bool save();
    static bool saveProjectToFile(const std::string& path);

private:
    static std::string s_currentProjectPath;  // FULL path to .trpgproj
    static std::string s_tempLoadPath;
};
