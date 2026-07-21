#pragma once

#include "core/LifecycleDevice.hpp"
#include "core/HardwareInterfaces.hpp"
#include "core/Types.hpp"

namespace STMHal
{

// ---------------------------------------------------------------------------
// RasterChannel
//
// Drives the raster mechanism's linear actuator: homing, moving to the two
// raster extremes, jogging, and probing an arbitrary position. Motion is
// only permitted while the device is ACTIVE (i.e. after initialize() +
// activate()); calling a motion method while INACTIVE/ERROR/SHUTDOWN
// returns Result::INVALID_STATE without touching the hardware. Any
// actuator failure during motion raises ERROR through the same path
// LifecycleDevice already uses for initialize/activate/etc.
// ---------------------------------------------------------------------------
class RasterChannel : public LifecycleDevice
{
public:

    /// @param actuator  Low-level motion hardware this channel drives. Not owned.
    explicit RasterChannel(ILinearActuator& actuator);

    /// Move to the calibrated home reference position.
    Result homePosition();

    /// Move to the calibrated left-most raster position.
    Result leftSide();

    /// Move to the calibrated right-most raster position.
    Result rightSide();

    /// Move to an arbitrary absolute position.
    Result probeAt(int position);

    /// Jog the actuator up by one increment.
    Result linearActuatorUp();

    /// Jog the actuator down by one increment.
    Result linearActuatorDown();

    /// True if the actuator is currently at the home reference position.
    bool isAtHome() const;

    /// Current absolute position, in the units defined by ILinearActuator.
    int currentPosition() const;

protected:

    Result onInitialize() override;
    Result onActivate()   override;
    Result onDeactivate() override;
    Result onRecover()    override;
    Result onShutdown()   override;

private:

    /// Interprets a driver result: raises ERROR on failure, otherwise
    /// passes the result through unchanged.
    Result commandMotion(Result driverResult);

    ILinearActuator& actuator_;

    // TODO: calibrate against the real mechanism's travel limits.
    static constexpr int kLeftPosition  = 0;
    static constexpr int kRightPosition = 100;
};

}
