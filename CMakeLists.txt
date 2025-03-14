cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 23)
project(NewRender LANGUAGES CXX)

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(app)
target_sources(app PRIVATE "src/main.cpp")
target_include_directories(app PRIVATE "${PROJECT_SOURCE_DIR}")

target_link_libraries(app PRIVATE fmt::fmt)
add_subdirectory("src/renderer")
target_link_libraries(app PRIVATE renderer)

include(CTest)
enable_testing()
