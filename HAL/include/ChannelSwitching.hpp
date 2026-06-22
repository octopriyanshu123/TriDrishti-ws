#pragma once

#include "core/LifecycleDevice.hpp"
#include "coreHardwareInterfaces.hpp"
#include "core/Types.hpp"

namespace STMHal
{

// ---------------------------------------------------------------------------
// ChannelSwitching
//
// Controls a single switchable electrical channel (relay, solenoid driver,
// SSR, etc). Maps the on/off concept directly onto the LifecycleDevice
// states: ACTIVE == channel energised, INACTIVE == channel de-energised.
// That means turnOn/turnOff get the existing error-detection and recovery
// machinery for free.
// ---------------------------------------------------------------------------
class ChannelSwitching : public LifecycleDevice
{
public:

    /// @param id      Logical identifier for this channel (logging / addressing).
    /// @param output  Low-level digital output this channel drives. Not owned —
    ///                caller is responsible for the output's lifetime.
    ChannelSwitching(ChannelId id, IDigitalOutput& output);

    /// Energise the channel. Equivalent to activate().
    Result turnOn();

    /// De-energise the channel. Equivalent to deactivate().
    Result turnOff();

    /// True if the channel is currently energised.
    bool isOn() const;

    ChannelId channelId() const { return channel_id_; }

protected:

    Result onInitialize() override;
    Result onActivate()   override;
    Result onDeactivate() override;
    Result onRecover()    override;
    Result onShutdown()   override;

private:

    ChannelId       channel_id_;
    IDigitalOutput& output_;
};

}
