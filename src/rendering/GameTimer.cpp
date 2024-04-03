#include"GameTimer.h"


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

