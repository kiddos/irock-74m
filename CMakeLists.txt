cmake_minimum_required(VERSION 3.16)
project(irock-74m CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(BUILD_SHARED_LIBS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_QUIET FALSE)
include(FetchContent)

set(BUILD_SHARED_LIBS OFF)

FetchContent_Declare(
  hidapi
  GIT_REPOSITORY https://github.com/libusb/hidapi.git
  GIT_TAG        hidapi-0.15.0
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(hidapi)

FetchContent_Declare(
  abseil
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG        20250512.0
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(abseil)
#
FetchContent_Declare(
  libevent
  GIT_REPOSITORY https://github.com/libevent/libevent.git
  GIT_TAG        release-2.1.12-stable
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  CMAKE_ARGS
      -DEVENT__DISABLE_DEBUG_MODE=ON
      -DEVENT__DISABLE_OPENSSL=ON # Set to OFF if you need OpenSSL support
      -DEVENT__DISABLE_REGRESSION_TEST=ON
      -DEVENT__DISABLE_SAMPLES=ON
      -DEVENT__DISABLE_BENCHMARK=ON
      -DEVENT__DISABLE_DOXYGEN=ON
)
FetchContent_MakeAvailable(libevent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG        1.12.0
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(cpr)

include_directories(
  ${hidapi_SOURCE_DIR}
  ${libevent_SOURCE_DIR}/include
  ${json_SOURCE_DIR}/include
)

add_executable(irock-74m-server server.cc irock_74m.cc)
target_link_libraries(irock-74m-server PRIVATE
  hidapi::libusb
  absl::log
  event_core
  event_extra
)

add_executable(irock-bot bot.cc)
target_link_libraries(irock-bot PRIVATE
  absl::strings
  cpr::cpr
)
