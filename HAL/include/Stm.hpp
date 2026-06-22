#pragma once

#include <array>
#include <cstddef>

#include "ChannelSwitching.hpp"
#include "RasterChannel.hpp"
#include "StmPeripheralStubs.hpp"

namespace hal
{

// ---------------------------------------------------------------------------
// Stm
//
// Top-level controller for the single STM32 board: owns every electrical
// channel plus the one raster mechanism. This is the only object main()
// needs to construct.
//
// The hardware driver members (StubDigitalOutput / StubLinearActuator) are
// placeholders — swap them for real STM32 GPIO / motion drivers once they
// exist. Everything else (ChannelSwitching, RasterChannel, and this class)
// stays the same, since they only depend on the IDigitalOutput /
// ILinearActuator interfaces.
// ---------------------------------------------------------------------------
class Stm
{
public:

    // TODO: set this to the actual number of electrical channels on the
    // board, and extend the initializer list in Stm.cpp to match.
    static constexpr std::size_t kChannelCount = 4;

    Stm();

    /// Initializes every channel and the raster mechanism.
    /// Stops at the first failure.
    Result initializeAll();

    /// Activates every channel and the raster mechanism.
    /// Stops at the first failure.
    Result activateAll();

    /// Deactivates every channel and the raster mechanism.
    /// Stops at the first failure.
    Result deactivateAll();

    /// Shuts everything down unconditionally (best-effort — every channel
    /// and the raster are shut down even if an earlier one fails).
    /// Returns the first failure encountered, if any.
    Result shutdownAll();

    /// @param index  Must be < kChannelCount.
    ChannelSwitching& channel(std::size_t index);

    RasterChannel& raster();

private:

    std::array<StubDigitalOutput, kChannelCount> channel_outputs_;
    std::array<ChannelSwitching, kChannelCount>  channels_;

    StubLinearActuator raster_actuator_;
    RasterChannel       raster_;
};

}
