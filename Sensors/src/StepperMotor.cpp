#include "HAL/actuator/StepperMotor.hpp"

namespace hal
{

Result StepperMotor::onInitialize()
{
    /*
        Open UART/CAN
        Configure driver
        Verify communication
    */

    return Result::SUCCESS;
}

Result StepperMotor::onActivate()
{
    /*
        Enable motor torque
    */

    return Result::SUCCESS;
}

Result StepperMotor::onDeactivate()
{
    /*
        Disable motor torque
    */

    return Result::SUCCESS;
}

Result StepperMotor::onShutdown()
{
    /*
        Close communication
    */

    return Result::SUCCESS;
}

Result StepperMotor::setVelocity(float velocity)
{
    if(getState() != State::ACTIVE)
    {
        return Result::INVALID_STATE;
    }

    target_velocity_ = velocity;

    return Result::SUCCESS;
}

Result StepperMotor::setPosition(float position)
{
    if(getState() != State::ACTIVE)
    {
        return Result::INVALID_STATE;
    }

    current_position_ = position;

    return Result::SUCCESS;
}

float StepperMotor::getPosition() const
{
    return current_position_;
}

}