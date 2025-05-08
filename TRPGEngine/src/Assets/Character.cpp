#include "Character.h"

Character::Character(const std::string& name)
    : m_name(name) {}

std::string Character::getName() const {
    return m_name;
}

json Character::toJson() const {
    return json{{"name", m_name}};
}

std::shared_ptr<Character> Character::fromJson(const json& j) {
    return std::make_shared<Character>(j.at("name"));
}