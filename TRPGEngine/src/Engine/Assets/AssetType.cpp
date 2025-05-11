#include "AssetType.h"
#include "Engine/Resources/ResourceManager.h"
#include "json.hpp"
#include <iostream>
#include <imgui.h>

static std::vector<AssetTypeInfo> assetTypeInfos = {
    {
        AssetType::Text, "text", {".txt", ".md"},
        [](const std::string& path) {
            std::cout << "Importing text file: " << path << std::endl;
            return true;
        }
    },
    {
        AssetType::Image, "image", {".png", ".jpg", ".jpeg", ".bmp"},
        [](const std::string& path) {
            std::cout << "Importing image file: " << path << std::endl;
            return true;
        }
    },
    {
        AssetType::Audio, "audio", {".mp3", ".wav", ".ogg"},
        [](const std::string& path) {
            std::cout << "Importing audio file: " << path << std::endl;
            return true;
        }
    },
    {
        AssetType::Video, "video", {".mp4", ".avi", ".mov"},
        [](const std::string& path) {
            std::cout << "Importing video file: " << path << std::endl;
            return true;
        }
    },
    {
        AssetType::Script, "script", {".lua", ".py"},
        [](const std::string& path) {
            std::cout << "Importing script file: " << path << std::endl;
            return true;
        }
    },
    {
        AssetType::Character, "character", {".json"},
        [](const std::string& path) {
            std::cout << "Importing character file: " << path << std::endl;
            return true;
        }
    }
};

const AssetTypeInfo* AssetTypeRegistry::getInfo(AssetType type) {
    for (const auto& info : assetTypeInfos)
        if (info.type == type) return &info;
    return nullptr;
}

const AssetTypeInfo* AssetTypeRegistry::getInfoByExtension(const std::string& ext) {
    for (const auto& info : assetTypeInfos)
        for (const auto& e : info.extensions)
            if (e == ext) return &info;
    return nullptr;
}

const std::vector<AssetTypeInfo>& AssetTypeRegistry::getAllTypes() {
    return assetTypeInfos;
}