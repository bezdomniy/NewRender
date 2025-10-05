#pragma once

#include <optional>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "../application/window.hpp"

class Compute;

class Device
{
private:
  struct SwapChainSupportDetails
  {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
  };

  struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;

    [[nodiscard]] auto isComplete() const -> bool
    {
      return graphicsFamily.has_value() && presentFamily.has_value()
          && computeFamily.has_value();
    }
  };

public:
  explicit Device(vk::Instance& instance);
  Device(vk::Instance& instance, vk::SurfaceKHR* surface);
  ~Device();
  void destroy() const;

  auto createCompute() -> Compute;

  auto createDescriptorPool(std::vector<vk::DescriptorPoolSize>& poolSizes,
                            uint32_t maxSets) -> vk::DescriptorPool;

  auto createComputePipeline(vk::PipelineShaderStageCreateInfo& stage,
                             vk::PipelineLayout& pipelineLayout,
                             vk::PipelineCache pipelineCache = nullptr) const
      -> vk::Pipeline;

  auto createGraphicsPipeline(
      vk::PipelineShaderStageCreateInfo& vertexStage,
      vk::PipelineShaderStageCreateInfo& fragmentStage,
      vk::PipelineVertexInputStateCreateInfo vertexInputInfo,
      vk::PipelineLayout& pipelineLayout,
      vk::PipelineCache pipelineCache = nullptr) const -> vk::Pipeline;

  auto createSwapchain(const Window& window)
      -> std::pair<vk::SwapchainKHR, vk::Extent2D>;
  auto getSwapchainImages(vk::SwapchainKHR swapchain) const
      -> std::vector<vk::Image>;
  auto getImageViews(std::vector<vk::Image>& images)
      -> std::vector<vk::ImageView>;

  vk::PhysicalDeviceMemoryProperties memoryProperties;
  vk::PhysicalDevice physicalDevice {VK_NULL_HANDLE};
  vk::Device handle {VK_NULL_HANDLE};
  vk::Queue graphicsQueue {VK_NULL_HANDLE};
  vk::Queue presentQueue {VK_NULL_HANDLE};
  vk::Queue computeQueue {VK_NULL_HANDLE};
  QueueFamilyIndices queueFamilyIndices;

private:
  vk::Instance& instance;
  vk::SurfaceKHR* surface = nullptr;
  vk::SurfaceFormatKHR surfaceFormat;

  auto querySwapChainSupport(vk::PhysicalDevice device)
      -> SwapChainSupportDetails;

  static auto chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR>& availableFormats)
      -> vk::SurfaceFormatKHR;
  static auto chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR>& availablePresentModes)
      -> vk::PresentModeKHR;
  static auto chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                               const Window& window) -> vk::Extent2D;

  void pickPhysicalDevice(std::vector<const char*>& deviceExtensions,
                          bool graphicsRequired = true);
  auto findQueueFamilies(vk::PhysicalDevice device,
                         bool graphicsRequired = true) -> QueueFamilyIndices;
  static auto checkDeviceExtensionSupport(
      vk::PhysicalDevice device,
      const std::vector<const char*>& deviceExtensions) -> bool;
};
