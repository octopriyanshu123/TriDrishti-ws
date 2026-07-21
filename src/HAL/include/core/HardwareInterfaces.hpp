#pragma once

#include "Result.hpp"

namespace STMHal
{

// ---------------------------------------------------------------------------
// IDigitalOutput
//
// Low-level abstraction over a single switchable electrical output (relay,
// solid-state relay, FET driver, etc). ChannelSwitching depends on this
// interface rather than any specific peripheral driver, so the same HAL
// logic works whether the output is wired through GPIO, an I2C relay board,
// or anything else — only the concrete implementation changes.
// ---------------------------------------------------------------------------
class IDigitalOutput
{
public:
    virtual ~IDigitalOutput() = default;

    virtual Result setOn(bool on) = 0;
    virtual bool   isOn() const = 0;
};

// ---------------------------------------------------------------------------
// ILinearActuator
//
// Low-level abstraction over the motion hardware that drives the raster
// mechanism. RasterChannel depends on this interface rather than talking to
// motor drivers / encoders directly.
// ---------------------------------------------------------------------------
class ILinearActuator
{
public:
    virtual ~ILinearActuator() = default;

    virtual Result moveUp()   = 0;
    virtual Result moveDown() = 0;
    virtual Result moveTo(int position) = 0;

    virtual bool isAtHome() const = 0;
    virtual int  currentPosition() const = 0;
};

}
