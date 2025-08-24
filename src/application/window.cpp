#include <string>
#include "window.hpp"
#include "SDL3/SDL_init.h"

Window::Window(const std::string& name, uint32_t width, uint32_t height)
    : properties({name, true, {width, height}})
{
  // Allow SIGINT to be handled by the application
  SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) == false) {
    SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
  } else {
    SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

    // Create window
    handle = SDL_CreateWindow("NewRender",
                              static_cast<int>(properties.extent.width),
                              static_cast<int>(properties.extent.height),
                              window_flags);
  }
}

Window::~Window()
{
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

  VkSurfaceKHR surface = VK_NULL_HANDLE;

  if (!SDL_Vulkan_CreateSurface(handle, instance, NULL, &surface)) {
    throw std::runtime_error(std::string("Failed to create window surface. ") + SDL_GetError());
  }

  return static_cast<vk::SurfaceKHR>(surface);
}

const Window::Extent& Window::getExtent() const
{
  return properties.extent;
}
