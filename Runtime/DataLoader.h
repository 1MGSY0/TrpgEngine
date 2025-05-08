#pragma once
#include <vector>
#include <string>

struct GameData {
    std::vector<std::string> texts;
    struct Character { std::string name; } ;
    std::vector<Character> characters;
    std::vector<std::string> audios;
};

class DataLoader {
public:
    static bool load(const std::string& path, GameData& outData);
};
