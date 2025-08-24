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
  explicit Device(vk::Instance& instance);
  Device(vk::Instance& instance, vk::SurfaceKHR* surface);
  ~Device();
  void destroy() const;

  Compute createCompute();

  struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;

    [[nodiscard]] bool isComplete() const
    {
      return graphicsFamily.has_value() && presentFamily.has_value()
          && computeFamily.has_value();
    }
  };

  vk::RenderPass createRenderPass() const;

  vk::DescriptorPool createDescriptorPool(
      std::vector<vk::DescriptorPoolSize>& poolSizes, uint32_t maxSets);

  vk::Pipeline createComputePipeline(
      vk::PipelineShaderStageCreateInfo& stage,
      vk::PipelineLayout& pipelineLayout,
      vk::PipelineCache pipelineCache = nullptr) const;

  vk::Pipeline createGraphicsPipeline(
      vk::PipelineShaderStageCreateInfo& vertexStage,
      vk::PipelineShaderStageCreateInfo& fragmentStage,
      vk::PipelineVertexInputStateCreateInfo vertexInputInfo,
      vk::RenderPass renderPass,
      vk::PipelineLayout& pipelineLayout,
      vk::PipelineCache pipelineCache = nullptr) const;

  vk::SwapchainKHR createSwapchain();
  std::vector<vk::Image> getSwapchainImages(vk::SwapchainKHR swapchain) const;
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

  static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR>& availableFormats);
  static vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR>& availablePresentModes);
  static vk::Extent2D chooseSwapExtent(
      const vk::SurfaceCapabilitiesKHR& capabilities);

  void pickPhysicalDevice(std::vector<const char*>& deviceExtensions,
                          bool graphicsRequired = true);
  QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device,
                                       bool graphicsRequired = true);
  static bool checkDeviceExtensionSupport(
      vk::PhysicalDevice device,
      const std::vector<const char*>& deviceExtensions);
};
