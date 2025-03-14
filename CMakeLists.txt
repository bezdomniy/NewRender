cmake_minimum_required(VERSION 3.14)
project(NewRender)

find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 23)

add_executable(NewRender main.cpp)

target_link_libraries(NewRender PRIVATE fmt::fmt Vulkan::Vulkan glm::glm glfw imgui::imgui)

include(CTest)
enable_testing()
