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

project(sync VERSION 0.0.3)
conan_cmake_run(
    CONANFILE conanfile.txt
    BASIC_SETUP CMAKE_TARGETS
    BUILD missing
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

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

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
file(GLOB SYNC_SOURCE_FILES src/sync.cpp)
file(GLOB SYNCD_SOURCE_FILES src/syncd.cpp)

list(REMOVE_ITEM SOURCE_FILES
    ${SYNC_SOURCE_FILES}
    ${SYNCD_SOURCE_FILES})

add_executable(sync
    ${SOURCE_FILES}
    ${SYNC_SOURCE_FILES}
#    ${STUB_HDRS}
    ${PROTO_SRCS}
    ${GRPC_SRCS})

add_executable(syncd
    ${SOURCE_FILES}
    ${SYNCD_SOURCE_FILES}
#    ${STUB_HDRS}
    ${PROTO_SRCS}
    ${GRPC_SRCS})

set(IMPORT_LIBS
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

target_link_libraries(sync PUBLIC ${IMPORT_LIBS})
target_link_libraries(syncd PUBLIC ${IMPORT_LIBS})

##########################################################################
enable_testing()
file(GLOB TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} tests/*.cpp)
foreach (TEST_SRC ${TEST_SRCS})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME}
        ${SOURCE_FILES}
        ${TEST_SRC}
        ${PROTO_SRCS}
        ${GRPC_SRCS})

    target_link_libraries(${TEST_NAME} ${IMPORT_LIBS})

    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests)

    add_test(NAME ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tests/${TEST_NAME})
endforeach (TEST_SRC)