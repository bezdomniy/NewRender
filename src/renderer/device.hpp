#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

class Device
{
public:
  Device(vk::Instance& instance);
  Device(vk::Instance& instance, vk::SurfaceKHR* surface);
  ~Device();
  void destroy();

  vk::SwapchainKHR createSwapchain();
  std::vector<vk::Image> getSwapchainImages(vk::SwapchainKHR swapchain);
  std::vector<vk::ImageView> getImageViews(vk::SwapchainKHR swapchain,
                                           std::vector<vk::Image>& images);

  vk::PhysicalDeviceMemoryProperties memoryProperties;
  vk::PhysicalDevice physicalDevice {VK_NULL_HANDLE};
  vk::Device handle {VK_NULL_HANDLE};
  vk::Queue graphicsQueue {VK_NULL_HANDLE};
  vk::Queue presentQueue {VK_NULL_HANDLE};
  vk::Queue computeQueue {VK_NULL_HANDLE};

private:
  struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;

    bool isComplete()
    {
      return graphicsFamily.has_value() && presentFamily.has_value()
          && computeFamily.has_value();
    }
  };

  struct SwapChainSupportDetails
  {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
  };

  vk::Instance& instance;
  vk::SurfaceKHR* surface = nullptr;
  QueueFamilyIndices queueFamilyIndices;

  SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);

  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR>& availableFormats);
  vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR>& availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

  void pickPhysicalDevice(std::vector<const char*>& deviceExtensions,
                          bool graphicsRequired = true);
  QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device,
                                       bool graphicsRequired = true);
  bool checkDeviceExtensionSupport(
      vk::PhysicalDevice device,
      const std::vector<const char*>& deviceExtensions);
};
