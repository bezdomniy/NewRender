cmake_minimum_required(VERSION 3.25)
include(CMakePrintHelpers)

set(CMAKE_CXX_STANDARD 23)
project(NewRender LANGUAGES CXX)

if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

# Fetch Slang binaries
include(cmake/FetchSlang.cmake)

include(cmake/openssf.cmake)

add_executable(app)
target_sources(app PRIVATE src/main.cpp)
target_include_directories(app PRIVATE "${PROJECT_SOURCE_DIR}")

add_subdirectory("src/shaders")
add_dependencies(app Shaders)

target_link_libraries(app PRIVATE fmt::fmt)
add_subdirectory("src/renderer")
target_link_libraries(app PRIVATE renderer)
add_subdirectory("src/application")
target_link_libraries(app PRIVATE application)


include(CTest)
enable_testing()
