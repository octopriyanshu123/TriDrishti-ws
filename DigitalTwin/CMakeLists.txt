cmake_minimum_required(VERSION 3.10)
project(MagneticRobot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)

add_executable(magnetic_robot src/main.cpp)

target_include_directories(magnetic_robot PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}   
)

target_link_libraries(magnetic_robot PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    m
)

target_compile_options(magnetic_robot PRIVATE -Wall -O2)

set_target_properties(magnetic_robot PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)



add_executable(ground_robot src/ground.cpp)

target_include_directories(ground_robot PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}   
)

target_link_libraries(ground_robot PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    m
)

target_compile_options(ground_robot PRIVATE -Wall -O2)

set_target_properties(ground_robot PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)