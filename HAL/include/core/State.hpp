#pragma once

namespace STMHal
{

enum class State
{
    UNINITIALIZED,
    INITIALIZING,
    INACTIVE,
    ACTIVE,
    ERROR,
    RECOVERING,
    SHUTDOWN
};

}
