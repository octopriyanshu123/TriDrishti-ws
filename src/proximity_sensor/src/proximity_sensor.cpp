#include "proximity_sensor.hpp"

namespace hardware
{

ProximitySensor::ProximitySensor()
    : object_detected_(false),
      distance_(0.0)
{
}

ProximitySensor::~ProximitySensor()
{
}

bool ProximitySensor::init()
{
    std::cout << "Proximity sensor initialized" << std::endl;
    return true;
}

bool ProximitySensor::isObjectDetected() const
{
    return object_detected_;
}

double ProximitySensor::getDistance() const
{
    return distance_;
}

void ProximitySensor::reset()
{
    object_detected_ = false;
    distance_ = 0.0;
}

}