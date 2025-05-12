#pragma once
#include <string>

class RuntimeLauncher {
public:
    static bool launch(const std::string& buildDir);
};
