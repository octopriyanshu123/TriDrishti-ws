cmake_minimum_required(VERSION 3.16)

project(proximity_sensor)

add_library(${PROJECT_NAME}
    src/proximity_sensor.cpp src/demo
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
)

target_compile_features(${PROJECT_NAME}
    PUBLIC
        cxx_std_17
)