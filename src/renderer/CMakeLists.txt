add_library(renderer STATIC)

# Add source files to the 'renderer' library
target_sources(renderer PRIVATE 
    renderer.cpp 
    renderer.hpp 
    buffer.cpp
    buffer.hpp
    validation.cpp 
    validation.hpp
)

# Add include directory for the renderer library
target_include_directories(renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(renderer PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

# Link the 'renderer' library with its dependencies
target_link_libraries(renderer PRIVATE Vulkan::Vulkan glm::glm glfw imgui::imgui fmt::fmt)
target_precompile_headers(renderer PRIVATE <vulkan/vulkan.hpp>)