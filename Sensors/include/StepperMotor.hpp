#pragma once

#include "HAL/core/LifecycleDevice.hpp"

namespace hal
{

class StepperMotor :
    public LifecycleDevice
{
public:

    Result setVelocity(float velocity);

    Result setPosition(float position);

    float getPosition() const;

protected:

    Result onInitialize() override;
    Result onActivate() override;
    Result onDeactivate() override;
    Result onShutdown() override;

private:

    float current_position_ = 0.0f;

    float target_velocity_ = 0.0f;
};

}