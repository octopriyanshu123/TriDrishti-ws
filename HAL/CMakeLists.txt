cmake_minimum_required(VERSION 3.16)
project(STMHal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# STMHal — the abstraction layer itself. Header-and-source pairs only; no
# concrete hardware drivers live in this library.
# ---------------------------------------------------------------------------
add_library(STMHal STATIC
    LifecycleDevice.cpp
    ChannelSwitching.cpp
    RasterChannel.cpp
    
)

target_include_directories(STMHal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(STMHal PRIVATE /W4)
else()
    target_compile_options(STMHal PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# hal_demo — exercises ChannelSwitching/RasterChannel against mock
# IDigitalOutput/ILinearActuator implementations, so the whole thing builds
# and runs without any real peripherals attached. Swap the mocks for real
# drivers once you have them.
# ---------------------------------------------------------------------------
add_executable(hal_demo
    demo/main.cpp
)

target_link_libraries(hal_demo PRIVATE STMHal)
