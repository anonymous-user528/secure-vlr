#pragma once

#include <chrono>

class Timer {
public:
    using Clock         = std::chrono::steady_clock;
    using DurationType  = Clock::duration;
    using TimePointType = Clock::time_point;

protected:
    DurationType  _elapsed;
    DurationType  _total_elapsed;
    TimePointType _last_update;

public:
    Timer(): _elapsed(DurationType(0)) {}

    void start() {
        _last_update = Clock::now();
    }

    void stop() {
        auto end = Clock::now();
        _elapsed = (end - _last_update);
        _total_elapsed += _elapsed;
    }
    
    DurationType elapsed() const {
        return _elapsed;
    }

    DurationType total_elapsed() const {
        return _total_elapsed;
    }
};


class TimerGuard {
protected:
    Timer& _timer;

public:
    TimerGuard(Timer& timer):_timer(timer) {
        _timer.start();
    }

    ~TimerGuard() {
        _timer.stop();
    }
};

