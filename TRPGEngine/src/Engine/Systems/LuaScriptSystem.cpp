#include "LuaScriptSystem.hpp"
#include "Engine/EntitySystem/Components/ScriptComponent.hpp"
#include <iostream>
#include <filesystem>

LuaScriptSystem& LuaScriptSystem::get() {
    static LuaScriptSystem inst;
    return inst;
}

LuaScriptSystem::LuaScriptSystem()
    : L(luaL_newstate()) {
    luaL_openlibs(L);
}

void LuaScriptSystem::init() {
    // Optional: preload scripts
    for (auto e : EntityManager::get().getAllEntities()) {
        auto sc = EntityManager::get().getComponent<ScriptComponent>(e);
        if (sc) {
            if (std::filesystem::exists(sc->scriptPath)) {
                if (luaL_dofile(L, sc->scriptPath.c_str())) {
                    std::cerr << "[Lua] Error loading " << sc->scriptPath
                              << ": " << lua_tostring(L, -1) << "\n";
                    lua_pop(L,1);
                }
            }
        }
    }
}

void LuaScriptSystem::runOnEvent(const std::string& eventName, Entity e) {
    auto sc = EntityManager::get().getComponent<ScriptComponent>(e);
    if (!sc) return;
    // Assume script defines function eventName(e)
    lua_getglobal(L, eventName.c_str());
    if (lua_isfunction(L, -1)) {
        lua_pushinteger(L, e);
        if (lua_pcall(L,1,0,0)) {
            std::cerr << "[Lua] Error running event " << eventName
                      << ": " << lua_tostring(L,-1) << "\n";
            lua_pop(L,1);
        }
    } else {
        lua_pop(L,1);
    }
}
