cmake_minimum_required(VERSION 3.9.3)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/toolchain.cmake)
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD
        "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.8/conan.cmake"
        "${CMAKE_BINARY_DIR}/conan.cmake"
        EXPECTED_MD5 58bc519cadc890b5c33235defadc1176)
endif ()
include(${CMAKE_BINARY_DIR}/conan.cmake)

project(backend VERSION 0.0.1)
conan_cmake_run(
    CONANFILE conanfile.txt
    BASIC_SETUP CMAKE_TARGETS
    BUILD missing
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_PUBLIC_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/public)

find_package(Threads REQUIRED)
#find_package(JsonRpc REQUIRED)
find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)

#set(STUB_DEST ${CMAKE_CURRENT_BINARY_DIR}/stubs)
#file(MAKE_DIRECTORY ${STUB_DEST})

set(PROTO_DEST ${CMAKE_CURRENT_BINARY_DIR}/protos)
file(MAKE_DIRECTORY ${PROTO_DEST})

#file(GLOB STUB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/stubs/*.json)
#jsonrpc_generate_cpp(STUB_HDRS ${STUB_DEST} ${STUB_FILES})

file(GLOB PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/protos/*.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_DEST} ${PROTO_FILES})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_DEST} ${PROTO_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CONAN_INCLUDE_DIRS})
link_directories(${CONAN_LIB_DIRS})

file(GLOB AGENT_RESOURCE_FILES
    ${CONAN_BIN_DIRS_AGENT}/*.js
    ${CONAN_BIN_DIRS_AGENT}/*.wasm)

file(GLOB EMBEDDER_SRCS src/embedder.cpp)
add_executable(embedder ${EMBEDDER_SRCS})
set(EMBEDDER_IMPORT_LIBS boost_filesystem boost_system fmt dl)
target_link_libraries(embedder PUBLIC ${EMBEDDER_IMPORT_LIBS})
add_custom_command(
    OUTPUT resources.cpp
    COMMAND embedder ${AGENT_RESOURCE_FILES}
    DEPENDS ${AGENT_RESOURCE_FILES})

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

file(GLOB SYNCAIDE_SRCS
    src/syncaide.cpp
    src/client/*.cpp
    src/rpc/*.cpp
    src/rpc/callers/*.cpp)

file(GLOB SYNCAIDED_SRCS
    src/syncaided.cpp
    src/server/*.cpp
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
#    ${STUB_HDRS}
    ${PROTO_SRCS}
    ${GRPC_SRCS})

add_executable(syncaided
    resources.cpp
    ${SOURCE_FILES}
    ${SYNCAIDED_SRCS}
#    ${STUB_HDRS}
    ${PROTO_SRCS}
    ${GRPC_SRCS})

set(SYNCAIDE_IMPORT_LIBS
    boost_program_options
    boost_log
    boost_filesystem
    boost_system
    boost_thread
    boost_date_time
    boost_unit_test_framework
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
    fmt)

target_link_libraries(syncaide PUBLIC ${SYNCAIDE_IMPORT_LIBS})
target_link_libraries(syncaided PUBLIC ${SYNCAIDE_IMPORT_LIBS})

### Tests ################################################################
enable_testing()

add_executable(test_peer
    tests/test_peer.cpp
    ${SOURCE_FILES}
    ${PROTO_SRCS}
    ${GRPC_SRCS})
target_link_libraries(test_peer ${SYNCAIDE_IMPORT_LIBS})
set_target_properties(test_peer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests)
add_test(NAME test_peer
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tests/test_peer)

add_executable(test_router
    tests/test_router.cpp
    resources.cpp
    ${SOURCE_FILES}
    ${SYNCAIDED_SRCS}
    ${PROTO_SRCS}
    ${GRPC_SRCS})
target_link_libraries(test_router ${SYNCAIDE_IMPORT_LIBS})
set_target_properties(test_router PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests)
add_test(NAME test_router
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tests/test_router)

add_executable(test_view
    tests/test_view.cpp
    ${SOURCE_FILES}
    ${PROTO_SRCS}
    ${GRPC_SRCS})
target_link_libraries(test_view ${SYNCAIDE_IMPORT_LIBS})
set_target_properties(test_view PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests)
add_test(NAME test_view
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tests/test_view)