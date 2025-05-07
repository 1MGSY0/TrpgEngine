#pragma once
#include <string>

class ProjectManager {
    public:
        static bool saveProject(const std::string& directory);
        static bool loadProject(const std::string& filePath);
    
        static void setCurrentProjectPath(const std::string& path);
        static std::string getCurrentProjectPath();
    
    private:
        static std::string s_currentProjectPath;
    };
