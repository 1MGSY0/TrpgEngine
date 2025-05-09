#include "AssetImporter.h"
#include <fstream>
#include <json.hpp>

std::shared_ptr<TextComponent> AssetImporter::importTextComponent(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return nullptr;

    nlohmann::json j;
    file >> j;
    file.close();

    auto comp = std::make_shared<TextComponent>();
    comp->fromJson(j);
    return comp;
}
