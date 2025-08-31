#pragma once
#include <string>
#include "Engine/EntitySystem/EntityManager.hpp"

class ProjectManager {
public:
    static std::string getCurrentProjectPath();
    static void setCurrentProjectPath(const std::string& path);

    static std::string getTempLoadPath();
    static void setTempLoadPath(const std::string& path);


    static bool CreateNewProject(const std::string& projectName, const std::string& projectPath);
    static bool loadProject(const std::string& path);
    static bool save();
    static bool saveProjectToFile(const std::string& filePath);

    static void ProjectManager::setProjectMetaEntity(Entity e) ;
    static Entity ProjectManager::getProjectMetaEntity() {return s_projectMetaEntity;}

    static void requestProjectInfoPrompt();     // set the latch
    static bool consumeProjectInfoPrompt();     // read+clear the latch

private:
    static Entity s_projectMetaEntity;
    static std::string s_currentProjectPath;  // FULL path to .trpgproj
    static std::string s_tempLoadPath;

    static inline bool s_needProjectInfoPrompt = false;  // default off
};
