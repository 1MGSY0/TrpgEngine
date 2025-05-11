#pragma once
#include <string>
#include <json.hpp>
#include "AssetType.h"

class AssetBase {
public:
    virtual ~AssetBase() = default;

    virtual std::string getID() const = 0;
    virtual nlohmann::json toJson() const = 0;
    virtual AssetType getType() const = 0; 
};