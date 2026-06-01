#pragma once

namespace hal
{

enum class Result
{
    SUCCESS,
    FAILURE,
    INVALID_STATE,
    TIMEOUT,
    NOT_CONNECTED,
    HARDWARE_ERROR
};

}