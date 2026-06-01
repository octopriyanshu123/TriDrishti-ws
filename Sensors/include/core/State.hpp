#pragma once

namespace hal
{

enum class State
{
    UNINITIALIZED,
    INITIALIZING,
    INACTIVE,
    ACTIVE,
    ERROR,
    SHUTDOWN
};

}