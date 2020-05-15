cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0042 NEW)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)
set (CMAKE_CXX_STANDARD 11)

set(SUBMODULES_NO_BUILD ON)

project (cellframe-sdk-nodejs)

add_definitions("-fpic")

set(CELLFRAME_MODULES "core chains mining network cs-dag-poa cs-dag-pos cs-none srv-vpn")

include_directories(${CMAKE_JS_INC})

include(../../deps/cellframe-sdk/cmake/OS_Detection.cmake)

add_subdirectory(../../deps/cellframe-sdk ./build/cellframe-sdk EXCLUDE_FROM_ALL)

file(GLOB SOURCE_FILES "src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_JS_LIB}
    cellframe-sdk
    dap_app_cli
)

# Define NAPI_VERSION
add_definitions(-DNAPI_VERSION=4)
