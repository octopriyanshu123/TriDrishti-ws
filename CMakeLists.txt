cmake_minimum_required(VERSION 3.22)

project(TriDrishti
    VERSION     1.0.0
    DESCRIPTION "TriDrishti Robot Control System"
    LANGUAGES   CXX
)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "[TriDrishti] Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "[TriDrishti] C++ standard: C++${CMAKE_CXX_STANDARD}")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)




find_package(Threads REQUIRED)


set(ROBOT_SOURCES
    src/main.cpp
    src/Robot.cpp
    src/Controllers/ControllerManager.cpp
    src/Controllers/STMController.cpp
    src/Controllers/ProcessManager.cpp

    src/State/Logger.cpp

)


add_executable(robot ${ROBOT_SOURCES} ${ROBOT_HEADERS})


target_include_directories(robot
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)


message(STATUS "--------------------------------------------")
message(STATUS "  Project  : ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  Build    : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install  : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Tests    : ${BUILD_TESTING}")
message(STATUS "--------------------------------------------")