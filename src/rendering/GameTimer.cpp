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
    float dur = std::chrono::duration_cast<std::chrono::milliseconds>(_clock.now() - _lastFrameTime).count() / 1000.f;
    _fps = 1 / dur;
    
    _totalFrame++;
    _totalDurSec += dur;
    _aveFps =_totalFrame / _totalDurSec;

    return dur;
}

