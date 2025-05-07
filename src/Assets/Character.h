#pragma once
#include <string>
#include <json.hpp> 

using json = nlohmann::json; 

class Character {
public:
    Character(const std::string& name);
    std::string getName() const;

    json toJson() const;
    static std::shared_ptr<Character> fromJson(const json& j);

private:
    std::string m_name;
};
