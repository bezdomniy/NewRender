#include "window.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

Window::Window(std::string name, uint32_t width, uint32_t height)
    : properties({name, true, {width, height}})
{
#if defined(VK_USE_PLATFORM_XLIB_KHR)
  glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, false);
#endif

  if (!glfwInit()) {
    throw std::runtime_error("GLFW couldn't be initialized.");
  }

  // glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  handle = glfwCreateWindow((int)properties.extent.width,
                            (int)properties.extent.height,
                            properties.title.c_str(),
                            nullptr,
                            nullptr);

  if (!handle) {
    throw std::runtime_error("Couldn't create glfw window.");
  }

  glfwSetInputMode(handle, GLFW_STICKY_KEYS, 1);
  glfwSetInputMode(handle, GLFW_STICKY_MOUSE_BUTTONS, 1);
}

Window::~Window()
{
  glfwDestroyWindow(handle);
  glfwTerminate();
}

auto Window::getExtensions(uint32_t* count) -> const char**
{
  return glfwGetRequiredInstanceExtensions(count);
}

auto Window::create_surface(vk::Instance instance) -> vk::SurfaceKHR
{
  if (instance == VK_NULL_HANDLE || !handle) {
    throw std::runtime_error("Instance or window handle is null.");
    return VK_NULL_HANDLE;
  }

  VkSurfaceKHR surface;

  auto result = glfwCreateWindowSurface(
      static_cast<VkInstance>(instance), handle, nullptr, &surface);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface.");
  }

  return static_cast<vk::SurfaceKHR>(surface);
}

const Window::Extent& Window::getExtent() const
{
  return properties.extent;
}
