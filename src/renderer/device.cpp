#include <cstdint>
#include <set>
#include <vector>

#include "device.hpp"

#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"

Device::Device(vk::Instance& instance)
    : Device(instance, nullptr)
{
}

Device::Device(vk::Instance& instance, vk::SurfaceKHR* surface)
    : instance(instance)
    , surface(surface)
{
  std::vector<const char*> enabledDeviceExtensions;

  if (surface) {
    enabledDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                               VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};
  }

  pickPhysicalDevice(enabledDeviceExtensions);

  queueFamilyIndices = findQueueFamilies(physicalDevice);

  memoryProperties = physicalDevice.getMemoryProperties();

  // create a Device
  float queuePriority = 0.0f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
      .queueFamilyIndex = surface ? queueFamilyIndices.graphicsFamily.value()
                                  : queueFamilyIndices.computeFamily.value(),
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};

#ifdef __APPLE__
#  ifndef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
#    define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
#  endif

  enabledDeviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

  vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature {
      .dynamicRendering = vk::True};

  vk::DeviceCreateInfo deviceCreateInfo {
      .pQueueCreateInfos = &deviceQueueCreateInfo,
      .queueCreateInfoCount = 1,
      .ppEnabledExtensionNames = enabledDeviceExtensions.data(),
      .enabledExtensionCount =
          static_cast<uint32_t>(enabledDeviceExtensions.size()),
      .pNext = &dynamicRenderingFeature};

  handle = physicalDevice.createDevice(deviceCreateInfo);

  handle.getQueue(queueFamilyIndices.computeFamily.value(), 0, &computeQueue);

  if (surface) {
    handle.getQueue(
        queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
    handle.getQueue(queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
  }
}

Device::~Device() = default;

void Device::destroy() const
{
  handle.destroy();
}

vk::DescriptorPool Device::createDescriptorPool(
    std::vector<vk::DescriptorPoolSize>& poolSizes, uint32_t maxSets)
{
  vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo {
      .maxSets = maxSets,
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.data()};
  return handle.createDescriptorPool(descriptorPoolCreateInfo);
}

vk::Pipeline Device::createComputePipeline(
    vk::PipelineShaderStageCreateInfo& stage,
    vk::PipelineLayout& pipelineLayout,
    vk::PipelineCache pipelineCache) const
{
  vk::ComputePipelineCreateInfo compute_pipeline_create_info {
      .stage = stage, .layout = pipelineLayout};

  vk::Result result;
  vk::Pipeline pipeline;
  std::tie(result, pipeline) =
      handle.createComputePipeline(pipelineCache, compute_pipeline_create_info);
  assert(result == vk::Result::eSuccess);

  return pipeline;
}

vk::Pipeline Device::createGraphicsPipeline(
    vk::PipelineShaderStageCreateInfo& vertexStage,
    vk::PipelineShaderStageCreateInfo& fragmentStage,
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo,
    vk::PipelineLayout& pipelineLayout,
    vk::PipelineCache pipelineCache) const
{
  std::array<vk::PipelineShaderStageCreateInfo, 2>
      pipelineShaderStageCreateInfos = {vertexStage, fragmentStage};

  // Additive blending
  vk::PipelineColorBlendAttachmentState blendAttachmentState(
      true,
      vk::BlendFactor::eOne,
      vk::BlendFactor::eOne,
      vk::BlendOp::eAdd,
      vk::BlendFactor::eSrcAlpha,
      vk::BlendFactor::eDstAlpha,
      vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
          | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

  vk::PipelineColorBlendStateCreateInfo colorBlendState {
      .logicOp = vk::LogicOp::eCopy,
      .pAttachments = &blendAttachmentState,
      .attachmentCount = 1};

  vk::PipelineDepthStencilStateCreateInfo depthStencilState;
  depthStencilState.depthTestEnable = false;
  depthStencilState.depthWriteEnable = false;
  depthStencilState.depthCompareOp = vk::CompareOp::eAlways;
  depthStencilState.back.compareOp = vk::CompareOp::eAlways;

  //  pipeline_cache,
  //  shader_stages,
  //  vertex_input_state,
  //  vk::PrimitiveTopology::ePointList,
  //  0,
  //  vk::PolygonMode::eFill,
  //  vk::CullModeFlagBits::eNone,
  //  vk::FrontFace::eCounterClockwise,
  //  {blend_attachment_state},
  //  depth_stencil_state,
  //  graphics.pipeline_layout,
  //  render_pass);

  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState {
      .topology = vk::PrimitiveTopology::ePointList};

  vk::PipelineTessellationStateCreateInfo tessellationState {
      .patchControlPoints = 0};

  vk::PipelineViewportStateCreateInfo viewportState {.viewportCount = 1,
                                                     .scissorCount = 1};

  // vk::PipelineRasterizationStateCreateInfo rasterization_state {
  //     .polygonMode = polygon_mode,
  //     .cullMode = cull_mode,
  //     .frontFace = front_face,
  //     .lineWidth = 1.0f};

  vk::PipelineRasterizationStateCreateInfo rasterizationState {
      .polygonMode = vk::PolygonMode::eFill, .lineWidth = 1.0f};

  // vk::PipelineMultisampleStateCreateInfo multisample_state {
  //     .rasterizationSamples = vk::SampleCountFlagBits::e1};

  vk::PipelineMultisampleStateCreateInfo multisampleState {
      .rasterizationSamples = vk::SampleCountFlagBits::e1};

  // std::array<vk::DynamicState, 2> dynamic_state_enables = {
  //     vk::DynamicState::eViewport, vk::DynamicState::eScissor};

  std::array<vk::DynamicState, 2> dynamicStateEnables(
      {vk::DynamicState::eViewport, vk::DynamicState::eScissor});

  // vk::PipelineDynamicStateCreateInfo dynamic_state {
  //     .dynamicStateCount =
  //     static_cast<uint32_t>(dynamic_state_enables.size()), .pDynamicStates =
  //     dynamic_state_enables.data()};

  vk::PipelineDynamicStateCreateInfo dynamicState {
      .dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size()),
      .pDynamicStates = dynamicStateEnables.data()};

  vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo {};
  pipelineRenderingCreateInfo.setColorAttachmentCount(1);
  pipelineRenderingCreateInfo.setColorAttachmentFormats(surfaceFormat.format);

  vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info {
      .pStages = pipelineShaderStageCreateInfos.data(),
      .pVertexInputState = &vertexInputInfo,  // pVertexInputState
      .pInputAssemblyState = &inputAssemblyState,  // pInputAssemblyState
      .pTessellationState = &tessellationState,  // pTessellationState
      .pViewportState = &viewportState,  // pViewportState
      .pRasterizationState = &rasterizationState,  // pRasterizationState
      .pMultisampleState = &multisampleState,  // pMultisampleState
      .pDepthStencilState = &depthStencilState,  // pDepthStencilState
      .pColorBlendState = &colorBlendState,  // pColorBlendState
      .pDynamicState = &dynamicState,  // pDynamicState,
      .layout = pipelineLayout,
      .renderPass = nullptr,  // renderPass
      .pNext = &pipelineRenderingCreateInfo};

  vk::Result result;
  vk::Pipeline pipeline;
  std::tie(result, pipeline) = handle.createGraphicsPipeline(
      pipelineCache, graphics_pipeline_create_info);
  assert(result == vk::Result::eSuccess);

  return pipeline;
}

std::pair<vk::SwapchainKHR, vk::Extent2D> Device::createSwapchain(
    const Window& window)
{
  SwapChainSupportDetails swapChainSupport =
      querySwapChainSupport(physicalDevice);
  vk::SurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);
  vk::PresentModeKHR presentMode =
      chooseSwapPresentMode(swapChainSupport.presentModes);
  vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0
      && imageCount > swapChainSupport.capabilities.maxImageCount)
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo {
      .surface = *surface,
      .minImageCount = imageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = swapChainSupport.capabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = presentMode,
      .clipped = VK_TRUE,
      .oldSwapchain = nullptr};

  auto indices = findQueueFamilies(physicalDevice);
  if (indices.graphicsFamily != indices.presentFamily) {
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                     indices.presentFamily.value()};
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }

  return std::pair {handle.createSwapchainKHR(createInfo), extent};
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

  std::vector<vk::ImageView> imagesViews;
  imagesViews.resize(images.size());
  for (size_t i = 0; i < images.size(); i++) {
    vk::ImageViewCreateInfo createInfo {
        .image = images[i],
        .viewType = vk::ImageViewType::e2D,
        .format = surfaceFormat.format,
        .components = vk::ComponentMapping(),
        .subresourceRange = vk::ImageSubresourceRange(
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)};

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
    const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window)
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    return capabilities.currentExtent;
  } else {
    int width, height;
    SDL_GetWindowSizeInPixels(window.handle, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width = std::clamp(actualExtent.width,
                                    capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height,
                                     capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);

    return vk::Extent2D {.height = actualExtent.height,
                         .width = actualExtent.width};
  }
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
