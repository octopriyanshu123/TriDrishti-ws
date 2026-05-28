#include "stepper_motor.hpp"

namespace hardware
{

StepperMotor::StepperMotor()
    : position_(0)
{
}

StepperMotor::~StepperMotor()
{
}

bool StepperMotor::init()
{
    std::cout << "Stepper motor initialized" << std::endl;
    return true;
}

void StepperMotor::resetPosition()
{
    position_ = 0;
}

int StepperMotor::getPosition() const
{
    return position_;
}

void StepperMotor::setPosition(int position)
{
    position_ = position;
}

}