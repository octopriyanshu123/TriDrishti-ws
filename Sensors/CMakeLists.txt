cmake_minimum_required(VERSION 3.16)
project(hal_lifecycle_sensor VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(linearActuator
    src/LifecycleDevice.cpp
    src/LinearActuator.cpp
)

target_include_directories(linearActuator
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        ${CMAKE_CURRENT_SOURCE_DIR}/include

)

target_compile_options(linearActuator PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

