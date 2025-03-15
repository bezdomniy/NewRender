add_library(application STATIC)

# Add source files to the 'application' library
target_sources(application PRIVATE 
    window.cpp 
    window.hpp
)

# Add include directory for the application library
target_include_directories(application PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the 'application' library with its dependencies
target_link_libraries(application PRIVATE glfw)