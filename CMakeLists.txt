cmake_minimum_required(VERSION 3.22)

project(TriDrishti LANGUAGES CXX)

add_subdirectory(src/i2w)
# add_subdirectory(imu)
# add_subdirectory(laser_profiler)
# add_subdirectory(lidar)
# add_subdirectory(linear_actuator)
# add_subdirectory(myactuator)
# add_subdirectory(ndt)
add_subdirectory(src/proximity_sensor)
add_subdirectory(src/stepper_motor)