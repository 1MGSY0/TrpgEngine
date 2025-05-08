#pragma once

class IPanel {
public:
    virtual ~IPanel() = default;
    virtual void render() = 0;
    virtual const char* getName() const = 0;
    virtual bool isVisible() const { return true; }
};
