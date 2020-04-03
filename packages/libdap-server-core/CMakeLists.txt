cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0042 NEW)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)
set (CMAKE_CXX_STANDARD 11)

project (libdap-server-core-nodejs)

add_definitions("-fpic")
add_definitions("-DDAP_LOG_MT")

include_directories(${CMAKE_JS_INC})
include_directories(../../deps/libdap/include)
include_directories(../../deps/libdap-crypto/include)
include_directories(../../deps/libdap-server-core/include)
include_directories(../common/include)

add_subdirectory(../../deps/libdap ./build/libdap)
add_subdirectory(../../deps/libdap-crypto ./build/libdap-crypto)
add_subdirectory(../../deps/libdap-server-core ./build/libdap-server-core)

file(GLOB SOURCE_FILES "src/*.cpp" "../common/cpp/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_JS_LIB}
    dap_core
    dap_crypto
    dap_server_core
)

# Define NAPI_VERSION
add_definitions(-DNAPI_VERSION=3)

