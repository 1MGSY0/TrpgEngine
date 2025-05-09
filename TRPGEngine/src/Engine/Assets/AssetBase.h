#pragma once
#include <string>
#include <json.hpp>

class AssetBase {
public:
    AssetBase() = default;
    AssetBase(const std::string& id) : m_id(id) {}
    virtual ~AssetBase() = default;

    virtual nlohmann::json toJson() const = 0;
    virtual void fromJson(const nlohmann::json& j) = 0;

    const std::string& getId() const { return m_id; }
    void setId(const std::string& id) { m_id = id; }

protected:
    std::string m_id;
};
