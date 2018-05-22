cmake_minimum_required(VERSION 3.9.3)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/toolchain.cmake)
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD
        "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.11/conan.cmake"
        "${CMAKE_BINARY_DIR}/conan.cmake")
endif ()
include(${CMAKE_BINARY_DIR}/conan.cmake)

project(backend VERSION 0.0.5)
conan_cmake_run(
    CONANFILE conanfile.txt
    BASIC_SETUP CMAKE_TARGETS
    BUILD missing
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_TESTS_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests)
set(CMAKE_UTILS_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/utils)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)

set(PROTO_DEST ${CMAKE_CURRENT_BINARY_DIR}/protos)
file(MAKE_DIRECTORY ${PROTO_DEST})

set(PROTOBUF_GENERATE_CPP_APPEND_PATH ON)
set(GRPC_GENERATE_CPP_APPEND_PATH ON)
file(GLOB AGENT_PROTO_FILES ${CONAN_AGENT_ROOT}/protos/*.proto)
file(GLOB BACKEND_PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/protos/*.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_DEST} ${BACKEND_PROTO_FILES} ${AGENT_PROTO_FILES})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_DEST} ${BACKEND_PROTO_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CONAN_INCLUDE_DIRS})
link_directories(${CONAN_LIB_DIRS})

### Embedder ##############################################################
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    file(GLOB AGENT_RESOURCE_FILES
        ${CONAN_BIN_DIRS_AGENT}/*.html
        ${CONAN_BIN_DIRS_AGENT}/*.js
        ${CONAN_BIN_DIRS_AGENT}/*.wasm)
else()
    file(GLOB AGENT_RESOURCE_FILES
        ${CONAN_BIN_DIRS_AGENT}/*.js
        ${CONAN_BIN_DIRS_AGENT}/*.wasm)
endif ()

file(GLOB EMBEDDER_SRCS src/embedder.cpp)
add_executable(embedder ${EMBEDDER_SRCS})
target_link_libraries(embedder PUBLIC
    boost_filesystem
    boost_system
    fmt
    dl)
set_target_properties(embedder PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_UTILS_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_UTILS_OUTPUT_DIRECTORY})
add_custom_command(
    OUTPUT resources.cpp
    COMMAND embedder ${AGENT_RESOURCE_FILES}
    DEPENDS ${AGENT_RESOURCE_FILES})

### Syncaide ##############################################################
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

file(GLOB SYNCAIDE_SRCS
    src/syncaide.cpp
    src/client/*.cpp
    src/rpc/*.cpp
    src/rpc/callers/*.cpp)

file(GLOB SYNCAIDED_SRCS
    src/syncaided.cpp
    src/server/*.cpp
    src/server/web/*.cpp
    src/rpc/*.cpp
    src/rpc/callers/*.cpp
    src/rpc/services/*.cpp)

list(REMOVE_ITEM SOURCE_FILES
    ${EMBEDDER_SRCS}
    ${SYNCAIDE_SRCS}
    ${SYNCAIDED_SRCS})

add_executable(syncaide
    ${SOURCE_FILES}
    ${SYNCAIDE_SRCS}
    ${PROTO_HDRS}
    ${PROTO_SRCS}
    ${GRPC_HDRS}
    ${GRPC_SRCS})

add_executable(syncaided
    resources.cpp
    ${SOURCE_FILES}
    ${SYNCAIDED_SRCS}
    ${PROTO_HDRS}
    ${PROTO_SRCS}
    ${GRPC_HDRS}
    ${GRPC_SRCS})

set(SYNCAIDE_IMPORT_LIBS
    boost_program_options
    boost_log
    boost_filesystem
    boost_system
    boost_thread
    boost_date_time
    boost_regex
    grpc
    grpc++
    grpc_unsecure
    grpc++_unsecure
    gpr
    cares
    protobuf
    bz2
    gflags_nothreads
    rt
    pthread
    ssl
    crypto
    dl
    z
    gcov
    fmt
    cryptopp)

target_link_libraries(syncaide PUBLIC ${SYNCAIDE_IMPORT_LIBS})
target_link_libraries(syncaided PUBLIC ${SYNCAIDE_IMPORT_LIBS})

### Tests #################################################################
enable_testing()

#-- test_peer -------------------------------------------------------------
add_executable(test_peer
    tests/test_peer.cpp
    src/peer.cpp)
target_link_libraries(test_peer PUBLIC
    boost_unit_test_framework)
set_target_properties(test_peer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_TESTS_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_TESTS_OUTPUT_DIRECTORY})
add_test(NAME test_peer
    WORKING_DIRECTORY ${CMAKE_TESTS_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_TESTS_OUTPUT_DIRECTORY}/test_peer)

#-- test_uri ---------------------------------------------------------
add_executable(test_uri
    tests/test_uri.cpp
    src/common/uri.cpp)
target_link_libraries(test_uri PUBLIC
    boost_unit_test_framework
    boost_regex)
set_target_properties(test_uri PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_TESTS_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_TESTS_OUTPUT_DIRECTORY})
add_test(NAME test_uri
    WORKING_DIRECTORY ${CMAKE_TESTS_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_TESTS_OUTPUT_DIRECTORY}/test_uri)

#-- test_view -------------------------------------------------------------
add_executable(test_view
    tests/test_view.cpp
    src/peer.cpp
    src/view.cpp)
target_link_libraries(test_view PUBLIC
    boost_unit_test_framework
    boost_system
    pthread)
set_target_properties(test_view PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_TESTS_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_TESTS_OUTPUT_DIRECTORY})
add_test(NAME test_view
    WORKING_DIRECTORY ${CMAKE_TESTS_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_TESTS_OUTPUT_DIRECTORY}/test_view)