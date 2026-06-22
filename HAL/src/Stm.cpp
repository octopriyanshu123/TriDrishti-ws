#include "Stm.hpp"

namespace hal
{

Stm::Stm()
    : channel_outputs_{}
    , channels_{
          ChannelSwitching(0, channel_outputs_[0]),
          ChannelSwitching(1, channel_outputs_[1]),
          ChannelSwitching(2, channel_outputs_[2]),
          ChannelSwitching(3, channel_outputs_[3])
      }
    , raster_actuator_{}
    , raster_(raster_actuator_)
{
}

Result Stm::initializeAll()
{
    for (auto& ch : channels_)
    {
        const Result result = ch.initialize();
        if (result != Result::SUCCESS)
        {
            return result;
        }
    }

    return raster_.initialize();
}

Result Stm::activateAll()
{
    for (auto& ch : channels_)
    {
        const Result result = ch.activate();
        if (result != Result::SUCCESS)
        {
            return result;
        }
    }

    return raster_.activate();
}

Result Stm::deactivateAll()
{
    for (auto& ch : channels_)
    {
        const Result result = ch.deactivate();
        if (result != Result::SUCCESS)
        {
            return result;
        }
    }

    return raster_.deactivate();
}

Result Stm::shutdownAll()
{
    Result overall = Result::SUCCESS;

    for (auto& ch : channels_)
    {
        const Result result = ch.shutdown();
        if (result != Result::SUCCESS)
        {
            overall = result;
        }
    }

    const Result raster_result = raster_.shutdown();
    if (raster_result != Result::SUCCESS)
    {
        overall = raster_result;
    }

    return overall;
}

ChannelSwitching& Stm::channel(std::size_t index)
{
    return channels_[index];
}

RasterChannel& Stm::raster()
{
    return raster_;
}

}
