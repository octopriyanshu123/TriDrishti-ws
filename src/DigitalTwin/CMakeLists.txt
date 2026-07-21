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



add_executable(oiltank src/oiltank.cpp)

target_include_directories(oiltank PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}   
)

target_link_libraries(oiltank PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    m
)

target_compile_options(oiltank PRIVATE -Wall -O2)

set_target_properties(oiltank PROPERTIES
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

add_executable(robot src/robot.cpp)

target_include_directories(robot PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}   
)

target_link_libraries(robot PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    m
)

target_compile_options(robot PRIVATE -Wall -O2)

set_target_properties(robot PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)




add_executable(ui src/ui.cpp)

target_include_directories(ui PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}   
)

target_link_libraries(ui PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    m
)

target_compile_options(ui PRIVATE -Wall -O2)

set_target_properties(ui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)









