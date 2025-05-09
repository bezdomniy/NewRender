#pragma once

#include <optional>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

class Compute;

class Device
{
public:
  Device(vk::Instance& instance);
  Device(vk::Instance& instance, vk::SurfaceKHR* surface);
  ~Device();
  void destroy();

  Compute createCompute();

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

  vk::DescriptorPool createDescriptorPool(
      std::vector<vk::DescriptorPoolSize>& poolSizes, uint32_t maxSets);

  vk::Pipeline createComputePipeline(vk::PipelineShaderStageCreateInfo& stage,
                                     vk::PipelineLayout& pipelineLayout,
                                     vk::PipelineCache pipelineCache = nullptr);

  vk::SwapchainKHR createSwapchain();
  std::vector<vk::Image> getSwapchainImages(vk::SwapchainKHR swapchain);
  std::vector<vk::ImageView> getImageViews(std::vector<vk::Image>& images);

  vk::PhysicalDeviceMemoryProperties memoryProperties;
  vk::PhysicalDevice physicalDevice {VK_NULL_HANDLE};
  vk::Device handle {VK_NULL_HANDLE};
  vk::Queue graphicsQueue {VK_NULL_HANDLE};
  vk::Queue presentQueue {VK_NULL_HANDLE};
  vk::Queue computeQueue {VK_NULL_HANDLE};
  QueueFamilyIndices queueFamilyIndices;

private:
  struct SwapChainSupportDetails
  {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
  };

  vk::Instance& instance;
  vk::SurfaceKHR* surface = nullptr;

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
