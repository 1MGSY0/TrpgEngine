#pragma once
#include <cstdint>
#include <vector> 
#include <string>      

using Entity = uint32_t;
constexpr Entity INVALID_ENTITY = 0;

enum class EntityType {
    Default,
    FlowNode,
    Dialogue,
    Background,
    UIButton,
    Character,
    Item,
    Folder,
};

