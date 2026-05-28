#pragma once

#include <iostream>

namespace hardware
{

class ProximitySensor
{
public:
    ProximitySensor();

    ~ProximitySensor();

    bool init();

    bool isObjectDetected() const;

    double getDistance() const;

    void reset();

private:
    bool object_detected_;
    double distance_;
};

}