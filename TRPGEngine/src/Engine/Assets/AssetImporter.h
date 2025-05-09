#pragma once
#include <string>
#include <memory>
#include "Engine/Entity/Components/TextComponent.h"

class AssetImporter {
public:
    static std::shared_ptr<TextComponent> importTextComponent(const std::string& filePath);
};
