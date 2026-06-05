# cmake_minimum_required(VERSION 3.16)
# project(HollowCylinder LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # ─────────────────────────────────────────────────────────
# #  Find required packages
# # ─────────────────────────────────────────────────────────

# find_package(OpenGL REQUIRED)   # ← only once (duplicate removed)
# find_package(GLEW   REQUIRED)
# find_package(glfw3  3.3 REQUIRED)
# find_package(glm    REQUIRED)
# find_package(GLUT   REQUIRED)   # ← moved here; no longer after the duplicate

# # ─────────────────────────────────────────────────────────
# #  Executables
# # ─────────────────────────────────────────────────────────
# add_executable(hollow_cylinder main.cpp)
# add_executable(robot           robot.cpp)

# # ─────────────────────────────────────────────────────────
# #  hollow_cylinder
# # ─────────────────────────────────────────────────────────
# target_include_directories(hollow_cylinder PRIVATE
#     ${OPENGL_INCLUDE_DIRS}
#     ${GLEW_INCLUDE_DIRS}
# )

# target_link_libraries(hollow_cylinder PRIVATE   # ← one coherent PRIVATE block
#     OpenGL::GL
#     GLEW::GLEW
#     glfw
#     glm::glm
# )

# # ─────────────────────────────────────────────────────────
# #  robot
# #  Uses GLFW + GLEW (modern) AND the legacy GLUT API,
# #  so all includes and libs are gathered in one block each.
# # ─────────────────────────────────────────────────────────
# target_include_directories(robot PRIVATE        # ← single merged block
#     ${OPENGL_INCLUDE_DIRS}
#     ${GLEW_INCLUDE_DIRS}
#     ${GLUT_INCLUDE_DIRS}
# )

# target_link_libraries(robot PRIVATE             # ← all-keyword (PRIVATE) — fixes mixed-signature error
#     OpenGL::GL
#     GLEW::GLEW
#     glfw
#     glm::glm
#     ${OPENGL_LIBRARIES}
#     ${GLUT_LIBRARIES}
#     m
# )

# # ─────────────────────────────────────────────────────────
# #  Platform tweaks
# # ─────────────────────────────────────────────────────────
# if(APPLE)
#     target_link_libraries(hollow_cylinder PRIVATE
#         "-framework Cocoa"
#         "-framework IOKit"
#         "-framework CoreVideo"
#     )
# endif()

# if(MSVC)
#     target_compile_definitions(hollow_cylinder PRIVATE _CRT_SECURE_NO_WARNINGS)
# endif()

# # ─────────────────────────────────────────────────────────
# #  Output directory
# # ─────────────────────────────────────────────────────────
# set_target_properties(hollow_cylinder robot PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
# )

# # ─────────────────────────────────────────────────────────
# #  Build-config summary
# # ─────────────────────────────────────────────────────────
# message(STATUS "")
# message(STATUS "  Hollow Cylinder — build configuration")
# message(STATUS "  ─────────────────────────────────────")
# message(STATUS "  Outer radius : 5.0  units")
# message(STATUS "  Thickness    : 0.3  units  (inner = 4.7)")
# message(STATUS "  Height       : 8.0  units")
# message(STATUS "  Top/Bottom   : OPEN (hollow)")
# message(STATUS "  Segments     : 120")
# message(STATUS "  Compiler     : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
# message(STATUS "")

cmake_minimum_required(VERSION 3.10)
project(CylinderRobot VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)

add_executable(cylinder_robot main.cpp)

target_include_directories(cylinder_robot PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
)

target_link_libraries(cylinder_robot PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    m
)

target_compile_options(cylinder_robot PRIVATE -Wall -O2)

set_target_properties(cylinder_robot PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

message(STATUS "Cylinder Robot – Robot drives on outer surface of hollow cylinder")
message(STATUS "Cylinder : R=${CYL_R}  H=15  |  Robot body 2x2x2  |  WheelR=0.5")