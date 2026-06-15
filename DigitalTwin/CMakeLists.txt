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




add_executable(corrdinare test/corrdinare.cpp)

target_include_directories(corrdinare PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}   
)

target_link_libraries(corrdinare PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    m
)

target_compile_options(corrdinare PRIVATE -Wall -O2)

set_target_properties(corrdinare PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


add_executable(motion test/motion.cpp)

target_include_directories(motion PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}   
)

target_link_libraries(motion PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    m
)

target_compile_options(motion PRIVATE -Wall -O2)

set_target_properties(motion PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)




add_executable(camera test/camera.cpp)

target_include_directories(camera PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}   
)

target_link_libraries(camera PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    m
)

target_compile_options(camera PRIVATE -Wall -O2)

set_target_properties(camera PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)



add_executable(oilTank src/core/oilTank.cpp)

target_include_directories(oilTank PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}   
)

target_link_libraries(oilTank PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    m
)

target_compile_options(oilTank PRIVATE -Wall -O2)

set_target_properties(oilTank PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

