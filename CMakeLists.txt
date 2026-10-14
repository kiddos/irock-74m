cmake_minimum_required(VERSION 3.16)
project(irock-74m CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_QUIET FALSE)
include(FetchContent)

FetchContent_Declare(
  hidapi
  GIT_REPOSITORY https://github.com/libusb/hidapi.git
  GIT_TAG        hidapi-0.15.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(hidapi)

FetchContent_Declare(
  abseil
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG        20250512.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(abseil)

set(EVENT__DISABLE_DEBUG_MODE ON CACHE BOOL "Disable libevent debug" FORCE)
set(EVENT__DISABLE_OPENSSL ON CACHE BOOL "Disable libevent openssl" FORCE)
set(EVENT__DISABLE_REGRESSION_TEST ON CACHE BOOL "Disable libevent regression tests" FORCE)
set(EVENT__DISABLE_SAMPLES ON CACHE BOOL "Disable libevent samples" FORCE)
set(EVENT__DISABLE_BENCHMARK ON CACHE BOOL "Disable libevent benchmark" FORCE)
set(EVENT__DISABLE_TESTS ON CACHE BOOL "Disable libevent tests" FORCE)
set(EVENT__DISABLE_REGRESS ON CACHE BOOL "Disable libevent regress" FORCE)
set(EVENT__DISABLE_DOXYGEN ON CACHE BOOL "Disable libevent doxygen" FORCE)
set(EVENT__LIBRARY_TYPE "STATIC" CACHE INTERNAL "")
FetchContent_Declare(
  libevent
  GIT_REPOSITORY https://github.com/libevent/libevent.git
  GIT_TAG        release-2.1.12-stable
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(libevent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.12.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG        1.12.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11
  GIT_TAG        v2.4.2
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(CLI11)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.90.8
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(imgui)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(irock-74m-server server.cc irock_74m.cc)
target_include_directories(irock-74m-server PRIVATE
  ${hidapi_SOURCE_DIR}
  ${libevent_SOURCE_DIR}/include
  ${json_SOURCE_DIR}/include
  ${CLI11_SOURCE_DIR}/include
)
target_link_libraries(irock-74m-server PRIVATE
  hidapi::libusb
  absl::log
  event_core
  event_extra
)

add_executable(irock-bot bot.cc)
target_include_directories(irock-bot PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${hidapi_SOURCE_DIR}
  ${json_SOURCE_DIR}/include
  ${CLI11_SOURCE_DIR}/include
)
target_link_libraries(irock-bot PRIVATE
  absl::strings
  cpr::cpr
)

add_executable(irock-ui
  ui.cc
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(irock-ui PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${hidapi_SOURCE_DIR}
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${json_SOURCE_DIR}/include
  ${CLI11_SOURCE_DIR}/include
)
target_link_libraries(irock-ui PRIVATE
  cpr::cpr
  glfw
  OpenGL::GL
  absl::log
)
