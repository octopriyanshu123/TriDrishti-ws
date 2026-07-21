#include "ChannelSwitching.hpp"

namespace STMHal
{

ChannelSwitching::ChannelSwitching(ChannelId id, IDigitalOutput& output)
    : channel_id_(id)
    , output_(output)
{
}

Result ChannelSwitching::turnOn()
{
    return activate();
}

Result ChannelSwitching::turnOff()
{
    return deactivate();
}

bool ChannelSwitching::isOn() const
{
    return isActive();
}

Result ChannelSwitching::onInitialize()
{
    // Start from a known, safe state: channel off until explicitly activated.
    return output_.setOn(false);
}

Result ChannelSwitching::onActivate()
{
    return output_.setOn(true);
}

Result ChannelSwitching::onDeactivate()
{
    return output_.setOn(false);
}

Result ChannelSwitching::onRecover()
{
    // Re-assert the safe (off) state before allowing re-activation.
    return output_.setOn(false);
}

Result ChannelSwitching::onShutdown()
{
    return output_.setOn(false);
}

}
