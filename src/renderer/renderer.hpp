#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "../application/window.hpp"
#include "buffer.hpp"

class Renderer
{
public:
  Renderer(std::string name, Window* window);
  ~Renderer();

  void createBuffer(std::string name,
                    void* data,
                    size_t size,
                    vk::BufferUsageFlags usageFlags,
                    vk::MemoryPropertyFlags memoryFlags);
  void run();

private:
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

  std::string appName;

  vk::Instance instance {VK_NULL_HANDLE};

  Window* window = nullptr;
  vk::SurfaceKHR surface {VK_NULL_HANDLE};
  vk::PhysicalDeviceMemoryProperties memoryProperties;
  vk::PhysicalDevice physicalDevice {VK_NULL_HANDLE};
  vk::Device device {VK_NULL_HANDLE};
  vk::Queue graphicsQueue {VK_NULL_HANDLE};
  vk::Queue presentQueue {VK_NULL_HANDLE};
  vk::CommandPool commandPool {VK_NULL_HANDLE};
  std::vector<vk::CommandBuffer> commandBuffers;
  vk::SwapchainKHR swapchain {VK_NULL_HANDLE};
  vk::Format swapchainImageFormat;
  vk::Extent2D swapchainExtent;
  std::vector<vk::Image> images;
  std::vector<vk::ImageView> imagesViews;
  std::unordered_map<std::string, Buffer> buffers;
  QueueFamilyIndices queueFamilyIndices;

#if !defined(NDEBUG)
  vk::DebugUtilsMessengerEXT debugUtilsMessenger {VK_NULL_HANDLE};
#endif

  void initVulkan();
  void render();
  void cleanup();

  void createInstance();
  void pickPhysicalDevice();
  void createDevice();

  QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
  SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);
  void pickPhysicalDevice(std::vector<const char*>& deviceExtensions);
  bool checkDeviceExtensionSupport(vk::PhysicalDevice device, const std::vector<const char*>& deviceExtensions);
};
