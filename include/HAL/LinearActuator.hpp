#pragma once

#include "LifecycleDevice.hpp"
#include <cstdint>
#include <functional>

// ---------------------------------------------------------------------------
// Feedback snapshot — pushed to subscribers on every poll cycle
// ---------------------------------------------------------------------------
struct LinearActuatorFeedback
{
    float    position_pct   { 0.0f };  ///< 0.0 – 100.0 %
    bool     is_moving      { false };
    bool     limit_min      { false }; ///< Lower limit switch triggered
    bool     limit_max      { false }; ///< Upper limit switch triggered
    bool     fault          { false }; ///< Driver-level fault
    bool     comms_ok       { true  }; ///< Last ModBus exchange successful
};

// ---------------------------------------------------------------------------
// LinearActuator
//
// HAL for a single linear actuator on the Jetson platform.
// Communicates with the STM layer over UART ModBus (115200 baud).
//
// Threading model: NOT owned here. The owner is responsible for calling
// pollFeedback() on a suitable periodic thread / timer.
// ---------------------------------------------------------------------------
class LinearActuator : public LifecycleDevice
{
public:

    // -----------------------------------------------------------------------
    // Construction
    // -----------------------------------------------------------------------

    /// @param device_id  ModBus slave address (matches STM firmware config)
    explicit LinearActuator(int device_id);
    ~LinearActuator() override;

    // -----------------------------------------------------------------------
    // Control API  (callable only while ACTIVE)
    // -----------------------------------------------------------------------

    /// Move to an absolute position expressed as a percentage [0.0, 100.0].
    /// Returns INVALID_STATE if not ACTIVE.
    Result setPosition(float percentage);

    /// Stop motion immediately (non-emergency — leaves device ACTIVE).
    Result stop();

    // -----------------------------------------------------------------------
    // Feedback API
    // -----------------------------------------------------------------------

    /// Latest cached feedback snapshot (always safe to call, any state).
    LinearActuatorFeedback getFeedback() const;

    /// Convenience helpers backed by the cached snapshot
    float  getPosition()           const;   ///< Current position [0–100 %]
    bool   isMoving()              const;
    bool   isLimitMinTriggered()   const;
    bool   isLimitMaxTriggered()   const;

    /// Register a callback invoked every time pollFeedback() receives new data.
    /// Pass nullptr to clear.
    using FeedbackCallback = std::function<void(const LinearActuatorFeedback&)>;
    void subscribeFeedback(FeedbackCallback cb);

    /// Call this periodically (e.g. from a timer thread at 10–50 Hz).
    /// Reads ModBus registers, updates the internal snapshot, fires callbacks,
    /// and raises an error if communication fails repeatedly.
    void pollFeedback();

    // -----------------------------------------------------------------------
    // Device identification
    // -----------------------------------------------------------------------
    int deviceId() const { return device_id_; }

protected:

    // -----------------------------------------------------------------------
    // LifecycleDevice hooks
    // -----------------------------------------------------------------------
    Result onInitialize() override;   ///< Open ModBus channel, verify device
    Result onActivate()   override;   ///< Enable actuator outputs
    Result onDeactivate() override;   ///< Disable outputs, hold position
    Result onRecover()    override;   ///< Reset ModBus + re-handshake STM
    Result onShutdown()   override;   ///< Safe-stop then close channel
    void   onError()      override;   ///< Log fault, stop motion

private:

    int                    device_id_;
    LinearActuatorFeedback feedback_;
    FeedbackCallback       feedback_cb_;

    int  comms_fail_count_   { 0 };
    static constexpr int kMaxCommsFailures = 5;  ///< Before raising ERROR

    // -----------------------------------------------------------------------
    // Internal ModBus helpers — implementation talks to the STM layer
    // -----------------------------------------------------------------------
    Result  modbusWritePosition(float percentage);
    Result  modbusWriteStop();
    Result  modbusReadFeedback(LinearActuatorFeedback& out);

    bool    validatePercentage(float pct) const;
};
