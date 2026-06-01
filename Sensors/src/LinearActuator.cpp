#include "LinearActuator.hpp"


LinearActuator::LinearActuator(int device_id)
    : device_id_(device_id)
{
}

LinearActuator::~LinearActuator()
{
    if (state() != State::SHUTDOWN)
        shutdown();
}

// ---------------------------------------------------------------------------
// LifecycleDevice hooks
// ---------------------------------------------------------------------------

Result LinearActuator::onInitialize()
{
    // 1. Open / verify UART ModBus channel to the STM at 115200 baud.
    // 2. Send an identification / ping request to device_id_.
    // 3. Populate an initial feedback snapshot.

    // --- STUB ---
    // Replace with: open_modbus_port(), send ID query, check response.
    comms_fail_count_ = 0;
    return Result::SUCCESS;
}

Result LinearActuator::onActivate()
{
    // Enable actuator driver outputs on the STM side.
    // --- STUB ---
    return Result::SUCCESS;
}

Result LinearActuator::onDeactivate()
{
    // Disable actuator driver outputs while keeping the channel open.
    // The actuator should hold its current position.
    // --- STUB ---
    return Result::SUCCESS;
}

Result LinearActuator::onRecover()
{
    // 1. Flush / reset the ModBus channel.
    // 2. Re-handshake with the STM device.
    // 3. Re-read hardware state.

    // --- STUB ---
    comms_fail_count_ = 0;
    return Result::SUCCESS;
}

Result LinearActuator::onShutdown()
{
    // Best-effort: tell the actuator to stop, then close the channel.
    modbusWriteStop();   // Ignore result — we are shutting down regardless.
    // --- STUB: close_modbus_port() ---
    return Result::SUCCESS;
}

void LinearActuator::onError()
{
    // Called whenever a lifecycle transition fails or raiseError() fires.
    // Suitable place for logging / alerting upper layers.
    // --- STUB: log_error("LinearActuator", device_id_, state()) ---
    modbusWriteStop();   // Best-effort safe-stop
}

// ---------------------------------------------------------------------------
// Control API
// ---------------------------------------------------------------------------

Result LinearActuator::setPosition(float percentage)
{
    if (state() != State::ACTIVE)
        return Result::INVALID_STATE;

    if (!validatePercentage(percentage))
        return Result::HARDWARE_FAULT;   // Out-of-range command

    return modbusWritePosition(percentage);
}

Result LinearActuator::stop()
{
    if (state() != State::ACTIVE)
        return Result::INVALID_STATE;

    return modbusWriteStop();
}

// ---------------------------------------------------------------------------
// Feedback API
// ---------------------------------------------------------------------------

LinearActuatorFeedback LinearActuator::getFeedback() const
{
    return feedback_;
}

float LinearActuator::getPosition() const
{
    return feedback_.position_pct;
}

bool LinearActuator::isMoving() const
{
    return feedback_.is_moving;
}

bool LinearActuator::isLimitMinTriggered() const
{
    return feedback_.limit_min;
}

bool LinearActuator::isLimitMaxTriggered() const
{
    return feedback_.limit_max;
}

void LinearActuator::subscribeFeedback(FeedbackCallback cb)
{
    feedback_cb_ = std::move(cb);
}

// ---------------------------------------------------------------------------
// pollFeedback — called by the owner's timer thread
// ---------------------------------------------------------------------------

void LinearActuator::pollFeedback()
{
    // Do nothing if the device isn't in an operational state
    if (state() != State::ACTIVE && state() != State::INACTIVE)
        return;

    LinearActuatorFeedback fresh;
    Result r = modbusReadFeedback(fresh);

    if (r != Result::SUCCESS)
    {
        ++comms_fail_count_;
        feedback_.comms_ok = false;

        if (comms_fail_count_ >= kMaxCommsFailures)
            raiseError();   // Escalate to lifecycle ERROR state

        return;
    }

    // Good read — reset failure counter and update snapshot
    comms_fail_count_ = 0;
    feedback_ = fresh;

    // Fire subscriber callback if registered
    if (feedback_cb_)
        feedback_cb_(feedback_);
}

// ---------------------------------------------------------------------------
// Internal ModBus helpers — STUBS
// Replace these with real ModBus RTU read/write operations.
// ---------------------------------------------------------------------------

Result LinearActuator::modbusWritePosition(float percentage)
{
    // Example: write holding register that maps to 0–10000 (0.01 % steps)
    // uint16_t raw = static_cast<uint16_t>(percentage * 100.0f);
    // return modbus_write_register(ctx_, device_id_, REG_TARGET_POS, raw)
    //        == 1 ? Result::SUCCESS : Result::TIMEOUT;
    (void)percentage;
    return Result::SUCCESS;
}

Result LinearActuator::modbusWriteStop()
{
    // Example: write a STOP coil / command register
    // return modbus_write_bit(ctx_, device_id_, COIL_STOP, 1)
    //        == 1 ? Result::SUCCESS : Result::TIMEOUT;
    return Result::SUCCESS;
}

Result LinearActuator::modbusReadFeedback(LinearActuatorFeedback& out)
{
    // Example: read a block of input registers starting at REG_FEEDBACK_BASE
    // uint16_t regs[8];
    // if (modbus_read_input_registers(ctx_, device_id_, REG_FEEDBACK_BASE, 8, regs) != 8)
    //     return Result::TIMEOUT;
    //
    // out.position_pct = regs[0] / 100.0f;
    // out.is_moving    = regs[1] & 0x01;
    // out.limit_min    = regs[1] & 0x02;
    // out.limit_max    = regs[1] & 0x04;
    // out.fault        = regs[1] & 0x08;
    // out.comms_ok     = true;
    (void)out;
    return Result::SUCCESS;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

bool LinearActuator::validatePercentage(float pct) const
{
    return pct >= 0.0f && pct <= 100.0f;
}


#include <iostream>
#include <fstream>
#include <ctime>
 
int main() {
 
    // Get current date and time
    std::time_t now = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
 
    // Append to log file
    std::ofstream log("/home/octobot/log/linearactuator.txt", std::ios::app);
    if (!log.is_open()) {
        std::cerr << "Failed to open log file.\n";
        return 1;
    }
 
    log << "[RUN] " << buf << "\n";
    log.close();
 
    std::cout << "Logged: " << buf << "\n";
 
    return 0;
}
 