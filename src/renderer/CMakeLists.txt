add_library(renderer STATIC)

# Add source files to the 'renderer' library
target_sources(renderer PRIVATE
    renderer.cpp
    renderer.hpp
    device.cpp
    device.hpp
    buffers/buffer.cpp
    buffers/buffer.hpp
    buffers/hostBuffer.cpp
    buffers/hostBuffer.hpp
    buffers/deviceBuffer.cpp
    buffers/deviceBuffer.hpp
    shader.cpp
    shader.hpp
    compute.cpp
    compute.hpp
    graphics.cpp
    graphics.hpp
    executor.cpp
    executor.hpp
    validation.cpp
    validation.hpp
)

# Add include directory for the renderer library
target_include_directories(renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(renderer PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

# Link the 'renderer' library with its dependencies
target_link_libraries(renderer PRIVATE
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    glm::glm
    SDL3::SDL3
    imgui::imgui
    fmt::fmt
)
target_precompile_headers(renderer PRIVATE <vulkan/vulkan.hpp>)
