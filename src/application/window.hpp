#pragma once

#include <cstdint>
#include <string>

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

class Window
{
public:
  Window(const std::string& name, uint32_t width, uint32_t height);
  ~Window();

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

  static auto getExtensions(uint32_t* count) -> const char**;
  [[nodiscard]] const Extent& getExtent() const;

private:
  Properties properties;
  GLFWwindow* handle = nullptr;
};
