#include "HAL/sensor/ProximitySensor.hpp"

namespace hal
{

Result ProximitySensor::onInitialize()
{
    return Result::SUCCESS;
}

Result ProximitySensor::onActivate()
{
    return Result::SUCCESS;
}

Result ProximitySensor::onDeactivate()
{
    return Result::SUCCESS;
}

Result ProximitySensor::onShutdown()
{
    return Result::SUCCESS;
}

bool ProximitySensor::isDetected() const
{
    return detected_;
}

}