#pragma once

#include <cstdint>

// ---------------------------------------------------------------------------
// Result — richer than bool, leaner than exceptions
// ---------------------------------------------------------------------------
enum class Result
{
    SUCCESS,
    TIMEOUT,
    CONNECTION_FAILED,
    INVALID_STATE,
    HARDWARE_FAULT
};

// ---------------------------------------------------------------------------
// State — mirrors real hardware lifecycle
// ---------------------------------------------------------------------------
enum class State
{
    UNINITIALIZED,   // Object created, nothing opened
    INITIALIZING,    // Opening ports, configuring hardware
    INACTIVE,        // Connected but outputs disabled
    ACTIVE,          // Fully operational
    ERROR,           // Hardware / communication failure
    RECOVERING,      // Attempting reconnection or reset
    SHUTDOWN         // Fully closed, resources released
};

// ---------------------------------------------------------------------------
// LifecycleDevice
//
// Base class for every hardware module in the HAL.
// Responsibilities: operational-state tracking + transition control ONLY.
// Hardware logic, communication protocol, and threading belong in subclasses.
// ---------------------------------------------------------------------------
class LifecycleDevice
{
public:

    virtual ~LifecycleDevice() = default;

    // -----------------------------------------------------------------------
    // Public transition API
    // Each call validates the current state before delegating to the virtual
    // on*() hook in the derived class.
    // -----------------------------------------------------------------------

    /// UNINITIALIZED → INITIALIZING → INACTIVE
    Result initialize();

    /// INACTIVE → ACTIVE
    Result activate();

    /// ACTIVE → INACTIVE
    Result deactivate();

    /// ERROR → RECOVERING → INACTIVE  (or back to ERROR on failure)
    Result recover();

    /// ANY → SHUTDOWN
    Result shutdown();

    // -----------------------------------------------------------------------
    // Observers
    // -----------------------------------------------------------------------
    State  state()      const { return current_state_; }
    bool   isActive()   const { return current_state_ == State::ACTIVE;   }
    bool   isInError()  const { return current_state_ == State::ERROR;    }
    bool   isShutdown() const { return current_state_ == State::SHUTDOWN; }

protected:

    // -----------------------------------------------------------------------
    // Override these in derived classes — pure virtual = must be implemented
    // -----------------------------------------------------------------------
    virtual Result onInitialize() = 0;   ///< Open port, detect device
    virtual Result onActivate()   = 0;   ///< Enable outputs / start feedback
    virtual Result onDeactivate() = 0;   ///< Disable outputs safely
    virtual Result onRecover()    = 0;   ///< Reset / reconnect hardware
    virtual Result onShutdown()   = 0;   ///< Release all resources

    /// Optional — override for custom error-entry behaviour (logging, alerts)
    virtual void   onError() {}

    /// Derived classes call this to signal a fault from within their logic
    void raiseError();

private:

    State current_state_ { State::UNINITIALIZED };

    void transitionTo(State s) { current_state_ = s; }
};
