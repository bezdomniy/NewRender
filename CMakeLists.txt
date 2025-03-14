cmake_minimum_required(VERSION 3.14)
project(NewRender LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()


find_package(fmt CONFIG REQUIRED)
add_executable(app)
target_sources(app PRIVATE "src/main.cpp")

target_link_libraries(app PRIVATE fmt::fmt)

add_subdirectory("src/renderer")

include(CTest)
enable_testing()
