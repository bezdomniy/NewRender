#pragma once

#include <cstdint>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

class Window
{
public:
  Window(const std::string& name, uint32_t width, uint32_t height);
  ~Window();

  SDL_Window* handle = nullptr;

  struct Extent
  {
    uint32_t width;
    uint32_t height;
  };

  struct Properties
  {
    std::string title;
    bool resizable = true;
    Extent extent = {1280, 720};
  };

  [[nodiscard]] auto create_surface(vk::Instance instance) const
      -> vk::SurfaceKHR;

  static auto getExtensions(uint32_t* count) -> const char *const *;
  [[nodiscard]] const Extent& getExtent() const;

private:
  Properties properties;
};
