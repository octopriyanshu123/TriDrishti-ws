#pragma once

#include "HAL/core/LifecycleDevice.hpp"

namespace hal
{

class ProximitySensor :
    public LifecycleDevice
{
public:

    bool isDetected() const;

protected:

    Result onInitialize() override;
    Result onActivate() override;
    Result onDeactivate() override;
    Result onShutdown() override;

private:

    bool detected_ = false;
};

}