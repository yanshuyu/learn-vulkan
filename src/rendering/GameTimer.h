#pragma once
#include<chrono>


class GameTimer
{
private:
    std::chrono::high_resolution_clock _clock{};
    std::chrono::high_resolution_clock::time_point _lastFrameTime{};
    
    float _totalDurSec{0};
    size_t _totalFrame{0};

    float _fps{0};
    float _aveFps{0};
    float _deltaTime{0};

public:
    GameTimer() { Reset(); }

    void Reset();
    void StartFrame();
    float EndFrame();
    float GetTotalSeconds() const { return _totalDurSec; }
    float GetFps() const { return _fps; }
    float GetAveFps() const { return _aveFps; }
    float GetDeltaTime() const { return _deltaTime; }
};
