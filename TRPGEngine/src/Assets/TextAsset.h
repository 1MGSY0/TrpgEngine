#pragma once
#include <string>
#include <json.hpp> 

using json = nlohmann::json; 

class TextAsset {
public:
    TextAsset(const std::string& name, const std::string& content);

    std::string getName() const;
    std::string getContent() const;
    void setContent(const std::string& newContent);

    json toJson() const;                      
    static std::shared_ptr<TextAsset> fromJson(const json& j);  // Deserialize from JSON

private:
    std::string m_name;
    std::string m_content;
};
