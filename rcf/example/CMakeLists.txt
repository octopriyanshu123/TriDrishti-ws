cmake_minimum_required(VERSION 3.16)

# Called via add_subdirectory — rcf::rcf already exists in parent scope.
# RCF_SRC_DIR exposes src/ so rcf_instantiate.cpp can include template bodies.

set(RCF_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

# Helper macro: one call per executable
#   add_rcf_example(target  source_dir  server_or_client.cpp)
macro(add_rcf_example TARGET DIR SRC)
    add_executable(${TARGET}
        ${DIR}/${SRC}
        ${DIR}/rcf_instantiate.cpp
    )
    target_include_directories(${TARGET} PRIVATE
        ${DIR}          # finds shared_types.hpp next to the source
        ${RCF_SRC_DIR}  # finds service/ and action/ sub-dirs for instantiate
    )
    target_link_libraries(${TARGET} PRIVATE rcf::rcf)
    set_target_properties(${TARGET} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endmacro()

# ── local service ──────────────────────────────────────────────────────────────
add_rcf_example(local_service_server  local/service  server.cpp)
add_rcf_example(local_service_client  local/service  client.cpp)

# ── local action ───────────────────────────────────────────────────────────────
add_rcf_example(local_action_server   local/action   server.cpp)
add_rcf_example(local_action_client   local/action   client.cpp)

# ── network service ────────────────────────────────────────────────────────────
add_rcf_example(network_service_server  network/service  server.cpp)
add_rcf_example(network_service_client  network/service  client.cpp)

# ── network action ─────────────────────────────────────────────────────────────
add_rcf_example(network_action_server   network/action   server.cpp)
add_rcf_example(network_action_client   network/action   client.cpp)
