#include "core/LifecycleDevice.hpp"

// ---------------------------------------------------------------------------
// initialize
// ---------------------------------------------------------------------------
Result LifecycleDevice::initialize()
{
    if (current_state_ != State::UNINITIALIZED)
        return Result::INVALID_STATE;

    transitionTo(State::INITIALIZING);

    Result r = onInitialize();
    if (r != Result::SUCCESS)
    {
        transitionTo(State::ERROR);
        onError();
        return r;
    }

    transitionTo(State::INACTIVE);
    return Result::SUCCESS;
}


Result LifecycleDevice::activate()
{
    if (current_state_ != State::INACTIVE)
        return Result::INVALID_STATE;

    Result r = onActivate();
    if (r != Result::SUCCESS)
    {
        transitionTo(State::ERROR);
        onError();
        return r;
    }

    transitionTo(State::ACTIVE);
    return Result::SUCCESS;
}

// ---------------------------------------------------------------------------
// deactivate
// ---------------------------------------------------------------------------
Result LifecycleDevice::deactivate()
{
    if (current_state_ != State::ACTIVE)
        return Result::INVALID_STATE;

    Result r = onDeactivate();
    if (r != Result::SUCCESS)
    {
        transitionTo(State::ERROR);
        onError();
        return r;
    }

    transitionTo(State::INACTIVE);
    return Result::SUCCESS;
}

// ---------------------------------------------------------------------------
// recover
// ---------------------------------------------------------------------------
Result LifecycleDevice::recover()
{
    if (current_state_ != State::ERROR)
        return Result::INVALID_STATE;

    transitionTo(State::RECOVERING);

    Result r = onRecover();
    if (r != Result::SUCCESS)
    {
        transitionTo(State::ERROR);
        onError();
        return r;
    }

    transitionTo(State::INACTIVE);
    return Result::SUCCESS;
}

// ---------------------------------------------------------------------------
// shutdown
// ---------------------------------------------------------------------------
Result LifecycleDevice::shutdown()
{
    // Allow shutdown from any state except already-shutdown
    if (current_state_ == State::SHUTDOWN)
        return Result::INVALID_STATE;

    onShutdown();           // Best-effort; ignore result (must not throw)
    transitionTo(State::SHUTDOWN);
    return Result::SUCCESS;
}

// ---------------------------------------------------------------------------
// raiseError  (called by derived class internals)
// ---------------------------------------------------------------------------
void LifecycleDevice::raiseError()
{
    transitionTo(State::ERROR);
    onError();
}
