#include "proximity_sensor.hpp"

#include <iostream>

int main()
{
    hardware::ProximitySensor sensor;

    sensor.init();

    std::cout << "Distance: "
              << sensor.getDistance()
              << std::endl;

    std::cout << "Object detected: "
              << sensor.isObjectDetected()
              << std::endl;

    sensor.reset();

    return 0;
}