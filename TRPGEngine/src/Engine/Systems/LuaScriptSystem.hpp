#pragma once
#include <lua.hpp>
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/ScriptComponent.hpp"

class LuaScriptSystem {
public:
    static LuaScriptSystem& get();
    void init();
    void runOnEvent(const std::string& eventName, Entity e);

private:
    LuaScriptSystem();
    lua_State* L;
};