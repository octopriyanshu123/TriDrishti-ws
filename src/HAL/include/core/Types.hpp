#pragma once

#include <cstdint>

namespace STMHal
{

/// Identifies a single electrical channel (e.g. relay output, solenoid driver).
using ChannelId = std::uint8_t;

/// Absolute position along the raster's travel, in whatever units the
/// concrete ILinearActuator implementation uses (steps, encoder counts, mm).
using Position = int;

}
