cmake_minimum_required(VERSION 3.16)

project(
rcf
VERSION 1.0.0
DESCRIPTION "Robot Control Framework"
LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(RCF_SOURCES
src/Logger.cpp
src/Serializer.cpp
src/transport/UnixTransport.cpp
src/transport/TcpTransport.cpp

src/service/ServiceServer.cpp
src/service/ServiceClient.cpp

src/action/GoalHandle.cpp
src/action/ActionServer.cpp
src/action/ActionClient.cpp
)

add_library(rcf SHARED ${RCF_SOURCES})

add_library(rcf::rcf ALIAS rcf)

target_include_directories(rcf
PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
$<INSTALL_INTERFACE:include>
)

target_compile_features(rcf PUBLIC cxx_std_17)

target_compile_options(rcf
PRIVATE
-Wall
-Wextra
-Wpedantic
-Wno-unused-parameter
)


target_link_libraries(rcf
PRIVATE
pthread
rt
)


include(GNUInstallDirs)
include(CMakePackageConfigHelpers)


install(
TARGETS rcf
EXPORT rcfTargets


ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}


)

install(
DIRECTORY include/
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)


install(
EXPORT rcfTargets
FILE rcfTargets.cmake
NAMESPACE rcf::
DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rcf
)


configure_package_config_file(
${CMAKE_CURRENT_SOURCE_DIR}/cmake/rcfConfig.cmake.in
${CMAKE_CURRENT_BINARY_DIR}/rcfConfig.cmake
INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rcf
)

write_basic_package_version_file(
${CMAKE_CURRENT_BINARY_DIR}/rcfConfigVersion.cmake
VERSION ${PROJECT_VERSION}
COMPATIBILITY SameMajorVersion
)

install(
FILES
${CMAKE_CURRENT_BINARY_DIR}/rcfConfig.cmake
${CMAKE_CURRENT_BINARY_DIR}/rcfConfigVersion.cmake
DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rcf
)

add_subdirectory(example)

