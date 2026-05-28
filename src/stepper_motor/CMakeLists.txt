cmake_minimum_required(VERSION 3.16)

project(stepper_motor)

add_executable(${PROJECT_NAME}
    src/stepper_motor.cpp src/demo.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
)