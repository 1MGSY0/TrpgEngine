#include "RuntimeLauncher.h"
#include <Windows.h>
#include <filesystem>
#include <iostream>

bool RuntimeLauncher::launch(const std::string& buildDir) {
    std::filesystem::path exePath = buildDir;
    exePath /= "RuntimePlayer.exe";  // Simulated runtime exe

    if (!std::filesystem::exists(exePath)) {
        std::cerr << "RuntimePlayer.exe not found in: " << exePath.parent_path() << "\n";
        return false;
    }

    // Launch runtime
    ShellExecuteA(NULL, "open", exePath.string().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return true;
}
