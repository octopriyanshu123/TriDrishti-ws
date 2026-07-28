cmake_minimum_required(VERSION 3.22)

project(Logger VERSION 1.0.0
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

# --- library -----------------------------------------------------------
add_library(logger STATIC
    src/logger.cpp
)

target_include_directories(logger
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)




target_link_libraries(logger
    PUBLIC
        Threads::Threads
)

# only link stdc++fs on older GCC where std::filesystem isn't merged into libstdc++
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(logger PUBLIC stdc++fs)
endif()

target_compile_options(logger PRIVATE
    -Wall -Wextra
)


# Install library
install(TARGETS logger
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY include/
    DESTINATION include
)

message(STATUS "[Logger] Test executable")

# --- test executable -----------------------------------------------------
set(test_logger_SRC
    src/main.cpp
)

add_executable(test_logger ${test_logger_SRC})

target_include_directories(test_logger
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_logger
    PRIVATE
        logger
)