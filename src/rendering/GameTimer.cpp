#include"GameTimer.h"

std::chrono::high_resolution_clock GameTimer::_clock{};
std::chrono::high_resolution_clock::time_point GameTimer::_lastFrameTime{};

float GameTimer::_totalDurSec = 0;
size_t GameTimer::_totalFrame = 0;

float GameTimer::_fps = 0;
float GameTimer::_aveFps = 0;
float GameTimer::_deltaTime = 0;

void GameTimer::Reset()
{
    _lastFrameTime = _clock.now();
    _totalDurSec = 0;
    _totalFrame = 0;
    _fps = 0;
    _aveFps = 0;
}

void GameTimer::StartFrame()
{
    _lastFrameTime = _clock.now();
    
}

float GameTimer::EndFrame()
{
    std::chrono::duration<double, std::milli> milsec = _clock.now() - _lastFrameTime;
    _deltaTime = milsec.count() / 1000.f;
    //float dur = std::chrono::duration_cast<std::chrono::milliseconds>(_clock.now() - _lastFrameTime).count() / 1000.f;
    _fps = 1.f / _deltaTime;
    
    _totalFrame++;
    _totalDurSec += _deltaTime;
    _aveFps =_totalFrame / _totalDurSec;
  
    return _deltaTime;
}

