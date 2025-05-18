#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

enum class FileAssetType {
    Unknown = 0,
    Entity,
    Prefab,
    Json,
    Image,
    Text,
    Model,
    Audio,
    // Extend as needed (e.g., Font, Material, Shader)
};

struct FileAssetTypeInfo {
    FileAssetType type;
    std::string name;
    std::vector<std::string> extensions;
};

// Inline registry of supported asset types
inline const std::vector<FileAssetTypeInfo>& getAllFileAssetTypes() {
    static std::vector<FileAssetTypeInfo> types = {
        { FileAssetType::Entity, "Entity Instance", { ".entity" }},
        { FileAssetType::Prefab, "Prefab Template", { ".prefab" }},
        { FileAssetType::Json, "Json", { ".json" } },
        { FileAssetType::Text, "Text", { ".txt" } },
        { FileAssetType::Image, "Image", { ".png", ".jpg", ".jpeg", ".tga", ".bmp" } },
        { FileAssetType::Model,   "Model",   { ".fbx", ".obj", ".gltf", ".glb" } },
        { FileAssetType::Audio,   "Audio",   { ".wav", ".mp3", ".ogg" } }
    };
    return types;
}

// Get type from file extension
inline FileAssetType getFileAssetTypeFromExtension(const std::string& ext) {
    for (const auto& typeInfo : getAllFileAssetTypes()) {
        for (const auto& e : typeInfo.extensions) {
            if (e == ext)
                return typeInfo.type;
        }
    }
    return FileAssetType::Unknown;
}

inline std::string getFileAssetTypeName(FileAssetType type) {
    for (const auto& typeInfo : getAllFileAssetTypes()) {
        if (typeInfo.type == type)
            return typeInfo.name;
    }
    return "Unknown";
}

inline std::string convertExtensionsToFilter(const std::vector<std::string>& extensions) {
    std::string filter;
    for (size_t i = 0; i < extensions.size(); ++i) {
        filter += "*." + extensions[i];
        if (i != extensions.size() - 1) filter += ";";
    }
    return filter;
}

