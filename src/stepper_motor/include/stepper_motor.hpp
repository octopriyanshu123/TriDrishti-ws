#pragma once

#include <iostream>

namespace hardware
{
class StepperMotor
{
public:
    StepperMotor();

    ~StepperMotor();

    bool init();

    void resetPosition();

    int getPosition() const;

    void setPosition(int position);

private:
    int position_;
};
}