#include "stepper_motor.hpp"

int main()
{
    hardware::StepperMotor motor;

    motor.init();

    motor.setPosition(100);

    std::cout << motor.getPosition() << std::endl;

    motor.resetPosition();

    std::cout << motor.getPosition() << std::endl;

    return 0;
}