#pragma once

#include "UI/IPanel.h"

class ProjectPanel : public IPanel {
public:
    void render() override;
    const char* getName() const override { return "Project"; }
};
