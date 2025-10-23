#pragma once

extern "C" {
    // Shared Editor Play controls implemented in FlowPlayTester.cpp
    bool Editor_Run_IsPlaying();
    void Editor_Run_Play();
    void Editor_Run_Stop();
    void Editor_Run_Restart();
}
