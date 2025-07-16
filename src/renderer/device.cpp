#include <set>
#include <vector>

#include "device.hpp"

#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

Device::Device(vk::Instance& instance)
    : instance(instance)
{
  std::vector<const char*> enabledDeviceExtensions {};
  pickPhysicalDevice(enabledDeviceExtensions, false);

  queueFamilyIndices = findQueueFamilies(physicalDevice, false);

  memoryProperties = physicalDevice.getMemoryProperties();

  // create a Device
  float queuePriority = 0.0f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo(
      vk::DeviceQueueCreateFlags(),
      queueFamilyIndices.computeFamily.value(),
      1,
      &queuePriority);

#ifdef __APPLE__
#  ifndef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
#    define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
#  endif

  enabledDeviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

  vk::DeviceCreateInfo deviceCreateInfo(vk::DeviceCreateFlags(),
                                        deviceQueueCreateInfo,
                                        {},
                                        enabledDeviceExtensions);

  handle = physicalDevice.createDevice(deviceCreateInfo);

  handle.getQueue(queueFamilyIndices.computeFamily.value(), 0, &computeQueue);
}

Device::Device(vk::Instance& instance, vk::SurfaceKHR* surface)
    : instance(instance)
    , surface(surface)
{
  std::vector<const char*> enabledDeviceExtensions {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  pickPhysicalDevice(enabledDeviceExtensions);

  queueFamilyIndices = findQueueFamilies(physicalDevice);

  memoryProperties = physicalDevice.getMemoryProperties();

  // create a Device
  float queuePriority = 0.0f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo(
      vk::DeviceQueueCreateFlags(),
      queueFamilyIndices.graphicsFamily.value(),
      1,
      &queuePriority);

#ifdef __APPLE__
#  ifndef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
#    define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
#  endif

  enabledDeviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

  vk::DeviceCreateInfo deviceCreateInfo(vk::DeviceCreateFlags(),
                                        deviceQueueCreateInfo,
                                        {},
                                        enabledDeviceExtensions);

  handle = physicalDevice.createDevice(deviceCreateInfo);

  handle.getQueue(queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
  handle.getQueue(queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
  handle.getQueue(queueFamilyIndices.computeFamily.value(), 0, &computeQueue);
}

Device::~Device() = default;

void Device::destroy() const
{
  handle.destroy();
}

vk::DescriptorPool Device::createDescriptorPool(
    std::vector<vk::DescriptorPoolSize>& poolSizes, uint32_t maxSets)
{
  return handle.createDescriptorPool({{}, maxSets, poolSizes});
}

vk::Pipeline Device::createComputePipeline(
    vk::PipelineShaderStageCreateInfo& stage,
    vk::PipelineLayout& pipelineLayout,
    vk::PipelineCache pipelineCache) const
{
  vk::ComputePipelineCreateInfo compute_pipeline_create_info(
      {}, stage, pipelineLayout);

  vk::Result result;
  vk::Pipeline pipeline;
  std::tie(result, pipeline) =
      handle.createComputePipeline(pipelineCache, compute_pipeline_create_info);
  assert(result == vk::Result::eSuccess);

  return pipeline;
}

vk::SwapchainKHR Device::createSwapchain()
{
  SwapChainSupportDetails swapChainSupport =
      querySwapChainSupport(physicalDevice);

  vk::SurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);
  vk::PresentModeKHR presentMode =
      chooseSwapPresentMode(swapChainSupport.presentModes);
  vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0
      && imageCount > swapChainSupport.capabilities.maxImageCount)
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo(
      vk::SwapchainCreateFlagsKHR(),
      *surface,
      imageCount,
      surfaceFormat.format,
      surfaceFormat.colorSpace,
      extent,
      1,
      vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eExclusive,
      nullptr,
      swapChainSupport.capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      presentMode,
      VK_TRUE,
      nullptr);

  auto indices = findQueueFamilies(physicalDevice);
  if (indices.graphicsFamily != indices.presentFamily) {
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                     indices.presentFamily.value()};
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }

  return handle.createSwapchainKHR(createInfo);
}

std::vector<vk::Image> Device::getSwapchainImages(
    vk::SwapchainKHR swapchain) const
{
  return handle.getSwapchainImagesKHR(swapchain);
}

std::vector<vk::ImageView> Device::getImageViews(std::vector<vk::Image>& images)
{
  SwapChainSupportDetails swapChainSupport =
      querySwapChainSupport(physicalDevice);

  vk::SurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);

  auto swapchainImageFormat = surfaceFormat.format;

  std::vector<vk::ImageView> imagesViews;
  imagesViews.resize(images.size());
  for (size_t i = 0; i < images.size(); i++) {
    vk::ImageViewCreateInfo createInfo(
        vk::ImageViewCreateFlags(),
        images[i],
        vk::ImageViewType::e2D,
        swapchainImageFormat,
        vk::ComponentMapping(),
        vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

    imagesViews[i] = handle.createImageView(createInfo);
  }
  return imagesViews;
}

Device::SwapChainSupportDetails Device::querySwapChainSupport(
    vk::PhysicalDevice device)
{
  SwapChainSupportDetails details;

  (void)device.getSurfaceCapabilitiesKHR(*surface, &details.capabilities);

  uint32_t formatCount;
  (void)device.getSurfaceFormatsKHR(*surface, &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    (void)device.getSurfaceFormatsKHR(
        *surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  (void)device.getSurfacePresentModesKHR(*surface, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    (void)device.getSurfacePresentModesKHR(
        *surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

vk::SurfaceFormatKHR Device::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
  for (const auto& availableFormat : availableFormats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb
        && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
    {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

vk::PresentModeKHR Device::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
  for (const auto& availablePresentMode : availablePresentModes) {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
      return availablePresentMode;
    }
  }

  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Device::chooseSwapExtent(
    const vk::SurfaceCapabilitiesKHR& capabilities)
{
  return capabilities.currentExtent;

  // // TODO: restore this
  //   if (capabilities.currentExtent.width !=
  //   std::numeric_limits<uint32_t>::max())
  //   {
  //     return capabilities.currentExtent;
  //   } else {
  //     auto [width, height] = window->getExtent();

  //     VkExtent2D actualExtent = {static_cast<uint32_t>(width),
  //                                static_cast<uint32_t>(height)};

  //     actualExtent.width = std::clamp(actualExtent.width,
  //                                     capabilities.minImageExtent.width,
  //                                     capabilities.maxImageExtent.width);
  //     actualExtent.height = std::clamp(actualExtent.height,
  //                                      capabilities.minImageExtent.height,
  //                                      capabilities.maxImageExtent.height);

  //     return actualExtent;
  //   }
}

bool Device::checkDeviceExtensionSupport(
    vk::PhysicalDevice device, const std::vector<const char*>& deviceExtensions)
{
  uint32_t extensionCount;
  (void)device.enumerateDeviceExtensionProperties(
      nullptr, &extensionCount, nullptr);

  std::vector<vk::ExtensionProperties> availableExtensions(extensionCount);
  (void)device.enumerateDeviceExtensionProperties(
      nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

Device::QueueFamilyIndices Device::findQueueFamilies(vk::PhysicalDevice device,
                                                     bool graphicsRequired)
{
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  device.getQueueFamilyProperties(&queueFamilyCount, nullptr);

  std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
  device.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());

  uint32_t i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
      indices.computeFamily = i;
    }

    if (graphicsRequired) {
      if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
        indices.graphicsFamily = i;
      }

      VkBool32 presentSupport = false;
      auto result = device.getSurfaceSupportKHR(i, *surface, &presentSupport);

      if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create window surface.");
      }

      if (presentSupport) {
        indices.presentFamily = i;
      }
    }

    if ((graphicsRequired && indices.isComplete())
        || (!graphicsRequired && indices.computeFamily.has_value()))
    {
      break;
    }

    i++;
  }

  return indices;
}

void Device::pickPhysicalDevice(std::vector<const char*>& deviceExtensions,
                                bool graphicsRequired)
{
  uint32_t deviceCount = 0;
  (void)instance.enumeratePhysicalDevices(&deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<vk::PhysicalDevice> devices(deviceCount);
  (void)instance.enumeratePhysicalDevices(&deviceCount, devices.data());

  for (const auto& device : devices) {
    bool extensionsSupported =
        checkDeviceExtensionSupport(device, deviceExtensions);

    bool swapChainAdequate = true;
    if (graphicsRequired) {
      swapChainAdequate = false;
      if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport =
            querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty()
            && !swapChainSupport.presentModes.empty();
      }
    }

    auto indices = findQueueFamilies(device, graphicsRequired);
    auto deviceSuitable =
        ((indices.isComplete() && swapChainAdequate)
         || (!graphicsRequired && indices.computeFamily.has_value()))
        && extensionsSupported;
    if (deviceSuitable) {
      physicalDevice = device;
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}
