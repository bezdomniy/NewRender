add_library(renderer STATIC)

# Add source files to the 'renderer' library
target_sources(renderer PRIVATE renderer.cpp renderer.hpp)

# Add include directory for the renderer library
target_include_directories(renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the 'renderer' library with its dependencies
target_link_libraries(renderer PRIVATE Vulkan::Vulkan glm::glm glfw imgui::imgui fmt::fmt)
