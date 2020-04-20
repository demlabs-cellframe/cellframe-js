cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0042 NEW)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)
set (CMAKE_CXX_STANDARD 11)

set(SUBMODULES_NO_BUILD ON)

project (cellframe-sdk-nodejs)

add_definitions("-fpic")
add_definitions("-DDAP_LOG_MT")


if(UNIX)
  add_definitions ("-DDAP_OS_UNIX")
endif()

include_directories(${CMAKE_JS_INC})
include_directories(../../deps/cellframe-sdk/libdap-crypto/include)
include_directories(../../deps/cellframe-sdk/libdap-server-core/include)
include_directories(../../deps/cellframe-sdk/libdap-chain-common/include)
include_directories(../../deps/cellframe-sdk/libdap-chain-mempool)
include_directories(../../deps/cellframe-sdk/libdap-chain)
include_directories(../../deps/cellframe-sdk/libdap-chain-global-db)
include_directories(../../deps/cellframe-sdk/libdap-chain-net)
include_directories(../../deps/cellframe-sdk/libdap-server-udp)
include_directories(../../deps/cellframe-sdk/libdap-server/http_server)
include_directories(../../deps/cellframe-sdk/libdap-server/enc_server)
include_directories(../../deps/cellframe-sdk/libdap-stream/stream)
include_directories(../../deps/cellframe-sdk/libdap-app-cli/include)

add_subdirectory(../../deps/cellframe-sdk ./build/cellframe-sdk EXCLUDE_FROM_ALL)

file(GLOB SOURCE_FILES "src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_JS_LIB}
    dap_core
    dap_crypto
    dap_server_core
    dap_udp_server
    dap_http_server
    dap_enc_server
    dap_stream
    dap_app_cli
)

# Define NAPI_VERSION
add_definitions(-DNAPI_VERSION=3)

