#include "window.hpp"
#include "SDL3/SDL_init.h"

Window::Window(const std::string& name, uint32_t width, uint32_t height)
    : properties({name, true, {width, height}})
{
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) == false) {
    SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
  } else {
    SDL_WindowFlags window_flags =
        SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE;

    // Create window
    handle = SDL_CreateWindow("NewRender",
                              static_cast<int>(properties.extent.width),
                              static_cast<int>(properties.extent.height),
                              window_flags);

    if (!handle) {
      SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
    } else {
      surface = SDL_GetWindowSurface(handle);
    }
  }
}

Window::~Window()
{
  // Clean up surface
  SDL_DestroySurface(surface);
  surface = nullptr;

  // Destroy window
  SDL_DestroyWindow(handle);
  handle = nullptr;

  // Quit SDL subsystems
  SDL_Quit();
}

auto Window::getExtensions(uint32_t* count) -> const char* const*
{
  return SDL_Vulkan_GetInstanceExtensions(count);
}

auto Window::create_surface(const vk::Instance instance) const -> vk::SurfaceKHR
{
  if (instance == VK_NULL_HANDLE || !handle) {
    throw std::runtime_error("Instance or window handle is null.");
    return VK_NULL_HANDLE;
  }

  VkSurfaceKHR surface;

  const auto result =
      SDL_Vulkan_CreateSurface(handle, instance, nullptr, &surface);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface.");
  }

  return static_cast<vk::SurfaceKHR>(surface);
}

const Window::Extent& Window::getExtent() const
{
  return properties.extent;
}
