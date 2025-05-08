#pragma once
#include <string>

class BuildSystem {
public:
    // Performs full build from projectPath → outputDirectory
    static bool buildProject(const std::string& projectPath, const std::string& outputDirectory);

private:
    static void copyAssets(const std::string& from, const std::string& to);
    static void copyRuntime(const std::string& to);
};
