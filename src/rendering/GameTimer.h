#pragma once
#include<chrono>


class GameTimer
{
private:
    static std::chrono::high_resolution_clock _clock;
    static std::chrono::high_resolution_clock::time_point _lastFrameTime;
    
    static float _totalDurSec;
    static size_t _totalFrame;

    static float _fps;
    static float _aveFps;
    static float _deltaTime;

public:
    GameTimer() = delete;

    static void Reset();
    static void StartFrame();
    static float EndFrame();
    static float GetTotalSeconds() { return _totalDurSec; }
    static float GetFps() { return _fps; }
    static float GetAveFps() { return _aveFps; }
    static float GetDeltaTime() { return _deltaTime; }
};
