cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0042 NEW)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)
set (CMAKE_CXX_STANDARD 11)

set(SUBMODULES_NO_BUILD ON)

project (libdap-stream-nodejs)

add_definitions("-fpic")
add_definitions("-DDAP_LOG_MT")


if(UNIX)
  add_definitions ("-DDAP_OS_UNIX")
endif()

include_directories(${CMAKE_JS_INC})
include_directories(../../deps/cellframe-sdk/libdap-crypto/include)
include_directories(../../deps/cellframe-sdk/libdap-server/http_server)
include_directories(../../deps/cellframe-sdk/libdap-stream/stream)
include_directories(../common/include)
include_directories(../libdap-server-core-nodejs/src)

add_subdirectory(../../deps/cellframe-sdk ./build/cellframe-sdk EXCLUDE_FROM_ALL)

file(GLOB SOURCE_FILES "src/*.cpp" "../common/cpp/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_JS_LIB}
    dap_stream
)

# Define NAPI_VERSION
add_definitions(-DNAPI_VERSION=3)

