#pragma once
#include "Engine/Entity/Entity.h"

class AudioComponent : public Entity {
public:
    AudioComponent() = default;
    AudioComponent(const std::string& id, const std::string& name)
        : Entity(id, name) {}

    void setAudioPath(const std::string& path) { m_audioPath = path; }
    const std::string& getAudioPath() const { return m_audioPath; }

    void setLoop(bool loop) { m_loop = loop; }
    bool isLooping() const { return m_loop; }

    void setVolume(float vol) { m_volume = vol; }
    float getVolume() const { return m_volume; }
    
    std::string& getPathRef() { return m_audioPath; }
    float& getVolumeRef() { return m_volume; }
    bool& getLoopRef() { return m_loop; }
    
    nlohmann::json toJson() const override {
        return {
            {"id", m_id},
            {"name", m_name},
            {"audioPath", m_audioPath},
            {"loop", m_loop},
            {"volume", m_volume}
        };
    }

    void fromJson(const nlohmann::json& j) override {
        m_id = j.at("id").get<std::string>();
        m_name = j.at("name").get<std::string>();
        m_audioPath = j.at("audioPath").get<std::string>();
        m_loop = j.at("loop").get<bool>();
        m_volume = j.at("volume").get<float>();
    }

private:
    std::string m_audioPath;
    bool m_loop = false;
    float m_volume = 1.0f;
};
