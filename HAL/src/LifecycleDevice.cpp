#include "core/LifecycleDevice.hpp"

namespace STMHal
{

Result LifecycleDevice::initialize()
{
    if (current_state_ != State::UNINITIALIZED)
    {
        return Result::INVALID_STATE;
    }

    transitionTo(State::INITIALIZING);
    const Result result = onInitialize();

    if (result == Result::SUCCESS)
    {
        transitionTo(State::INACTIVE);
    }
    else
    {
        transitionTo(State::ERROR);
        onError();
    }

    return result;
}

Result LifecycleDevice::activate()
{
    if (current_state_ != State::INACTIVE)
    {
        return Result::INVALID_STATE;
    }

    const Result result = onActivate();

    if (result == Result::SUCCESS)
    {
        transitionTo(State::ACTIVE);
    }
    else
    {
        transitionTo(State::ERROR);
        onError();
    }

    return result;
}

Result LifecycleDevice::deactivate()
{
    if (current_state_ != State::ACTIVE)
    {
        return Result::INVALID_STATE;
    }

    const Result result = onDeactivate();

    if (result == Result::SUCCESS)
    {
        transitionTo(State::INACTIVE);
    }
    else
    {
        transitionTo(State::ERROR);
        onError();
    }

    return result;
}

Result LifecycleDevice::recover()
{
    if (current_state_ != State::ERROR)
    {
        return Result::INVALID_STATE;
    }

    transitionTo(State::RECOVERING);
    const Result result = onRecover();

    if (result == Result::SUCCESS)
    {
        transitionTo(State::INACTIVE);
    }
    else
    {
        transitionTo(State::ERROR);
        onError();
    }

    return result;
}

Result LifecycleDevice::shutdown()
{
    if (current_state_ == State::SHUTDOWN)
    {
        return Result::SUCCESS;
    }

    const Result result = onShutdown();

    // Shutdown is terminal and unconditional: resources must be released
    // regardless of what the hardware reports, so the device never gets
    // stuck mid-teardown. The returned Result still tells the caller
    // whether the hardware cooperated.
    transitionTo(State::SHUTDOWN);

    return result;
}

void LifecycleDevice::raiseError()
{
    transitionTo(State::ERROR);
    onError();
}

}
