#pragma once
#include <string>
#include <json.hpp> 

using json = nlohmann::json; 

class AudioAsset {
public:
    AudioAsset(const std::string& name, const std::string& filePath);

    std::string getName() const;
    std::string getFilePath() const;

    json toJson() const;
    static std::shared_ptr<AudioAsset> fromJson(const json& j);

private:
    std::string m_name;
    std::string m_filePath;
};
