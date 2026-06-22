#include "RasterChannel.hpp"

namespace STMHal
{

RasterChannel::RasterChannel(ILinearActuator& actuator)
    : actuator_(actuator)
{
}

Result RasterChannel::commandMotion(Result driverResult)
{
    if (driverResult != Result::SUCCESS)
    {
        raiseError();
    }

    return driverResult;
}

Result RasterChannel::homePosition()
{
    if (!isActive())
    {
        return Result::INVALID_STATE;
    }

    return commandMotion(actuator_.moveTo(0));
}

Result RasterChannel::leftSide()
{
    if (!isActive())
    {
        return Result::INVALID_STATE;
    }

    return commandMotion(actuator_.moveTo(kLeftPosition));
}

Result RasterChannel::rightSide()
{
    if (!isActive())
    {
        return Result::INVALID_STATE;
    }

    return commandMotion(actuator_.moveTo(kRightPosition));
}

Result RasterChannel::probeAt(int position)
{
    if (!isActive())
    {
        return Result::INVALID_STATE;
    }

    return commandMotion(actuator_.moveTo(position));
}

Result RasterChannel::linearActuatorUp()
{
    if (!isActive())
    {
        return Result::INVALID_STATE;
    }

    return commandMotion(actuator_.moveUp());
}

Result RasterChannel::linearActuatorDown()
{
    if (!isActive())
    {
        return Result::INVALID_STATE;
    }

    return commandMotion(actuator_.moveDown());
}

bool RasterChannel::isAtHome() const
{
    return actuator_.isAtHome();
}

int RasterChannel::currentPosition() const
{
    return actuator_.currentPosition();
}

Result RasterChannel::onInitialize()
{
    // Establish a known reference point before allowing motion commands.
    return actuator_.moveTo(0);
}

Result RasterChannel::onActivate()
{
    // Activation just permits motion commands to be issued; no motion
    // happens by itself. Override here if your hardware needs an explicit
    // "enable motor" step before it will move.
    return Result::SUCCESS;
}

Result RasterChannel::onDeactivate()
{
    // Hold the current position; no motion required to go inactive.
    return Result::SUCCESS;
}

Result RasterChannel::onRecover()
{
    // Re-home after a fault before resuming normal operation.
    return actuator_.moveTo(0);
}

Result RasterChannel::onShutdown()
{
    // Return to home and leave the actuator in a safe, known position.
    return actuator_.moveTo(0);
}

}
