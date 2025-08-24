#include <cstdint>
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

vk::Pipeline Device::createGraphicsPipeline(
    vk::PipelineShaderStageCreateInfo& vertexStage,
    vk::PipelineShaderStageCreateInfo& fragmentStage,
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo,
    vk::RenderPass renderPass,
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

  vk::PipelineColorBlendStateCreateInfo colorBlendState(
      vk::PipelineColorBlendStateCreateFlags(),
      false,
      vk::LogicOp::eCopy,
      blendAttachmentState,
      {0.0f, 0.0f, 0.0f, 0.0f});

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

  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState(
      vk::PipelineInputAssemblyStateCreateFlags(),
      vk::PrimitiveTopology::ePointList);

  vk::PipelineTessellationStateCreateInfo tessellationState(
      vk::PipelineTessellationStateCreateFlags(), 0);

  vk::PipelineViewportStateCreateInfo viewportState(
      vk::PipelineViewportStateCreateFlags(), 1, nullptr, 1, nullptr);

  // vk::PipelineRasterizationStateCreateInfo rasterization_state {
  //     .polygonMode = polygon_mode,
  //     .cullMode = cull_mode,
  //     .frontFace = front_face,
  //     .lineWidth = 1.0f};

  vk::PipelineRasterizationStateCreateInfo rasterizationState(
      vk::PipelineRasterizationStateCreateFlags(),
      false,
      false,
      vk::PolygonMode::eFill,
      vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise,
      false,
      0.0f,
      0.0f,
      0.0f,
      1.0f);

  // vk::PipelineMultisampleStateCreateInfo multisample_state {
  //     .rasterizationSamples = vk::SampleCountFlagBits::e1};

  vk::PipelineMultisampleStateCreateInfo multisampleState(
      vk::PipelineMultisampleStateCreateFlags(), vk::SampleCountFlagBits::e1);

  // vk::PipelineColorBlendStateCreateInfo color_blend_state {
  //     .attachmentCount =
  //     static_cast<uint32_t>(blend_attachment_states.size()), .pAttachments =
  //     blend_attachment_states.data()};

  vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo(
      vk::PipelineColorBlendStateCreateFlags(),
      false,
      vk::LogicOp::eCopy,
      1,
      &blendAttachmentState,
      {0.0f, 0.0f, 0.0f, 0.0f});

  // std::array<vk::DynamicState, 2> dynamic_state_enables = {
  //     vk::DynamicState::eViewport, vk::DynamicState::eScissor};

  std::array<vk::DynamicState, 2> dynamicStateEnables(
      {vk::DynamicState::eViewport, vk::DynamicState::eScissor});

  // vk::PipelineDynamicStateCreateInfo dynamic_state {
  //     .dynamicStateCount =
  //     static_cast<uint32_t>(dynamic_state_enables.size()), .pDynamicStates =
  //     dynamic_state_enables.data()};

  vk::PipelineDynamicStateCreateInfo dynamicState(
      vk::PipelineDynamicStateCreateFlags(),
      static_cast<uint32_t>(dynamicStateEnables.size()),
      dynamicStateEnables.data());

  vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info(
      vk::PipelineCreateFlags(),
      pipelineShaderStageCreateInfos,
      &vertexInputInfo,  // pVertexInputState
      &inputAssemblyState,  // pInputAssemblyState
      &tessellationState,  // pTessellationState
      &viewportState,  // pViewportState
      &rasterizationState,  // pRasterizationState
      &multisampleState,  // pMultisampleState
      &depthStencilState,  // pDepthStencilState
      &colorBlendState,  // pColorBlendState
      &dynamicState,  // pDynamicState,
      pipelineLayout,
      renderPass,  // renderPass
      0,  // subpass
      nullptr);

  vk::Result result;
  vk::Pipeline pipeline;
  std::tie(result, pipeline) = handle.createGraphicsPipeline(
      pipelineCache, graphics_pipeline_create_info);
  assert(result == vk::Result::eSuccess);

  return pipeline;
}

vk::RenderPass Device::createRenderPass() const
{
  // std::array<vk::AttachmentDescription, 2> attachments {
  //     {// Color attachment
  //      {.format = get_render_context().get_format(),
  //       .samples = vk::SampleCountFlagBits::e1,
  //       .loadOp = vk::AttachmentLoadOp::eClear,
  //       .storeOp = vk::AttachmentStoreOp::eStore,
  //       .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
  //       .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
  //       .initialLayout = vk::ImageLayout::eUndefined,
  //       .finalLayout = vk::ImageLayout::ePresentSrcKHR},
  //      // Depth attachment
  //      {.format = depth_format,
  //       .samples = vk::SampleCountFlagBits::e1,
  //       .loadOp = vk::AttachmentLoadOp::eClear,
  //       .storeOp = vk::AttachmentStoreOp::eDontCare,
  //       .stencilLoadOp = vk::AttachmentLoadOp::eClear,
  //       .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
  //       .initialLayout = vk::ImageLayout::eUndefined,
  //       .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal}}};

  std::array<vk::AttachmentDescription, 2> attachments {
      {// Color attachment
       {vk::AttachmentDescriptionFlags(),
        vk::Format::eB8G8R8A8Unorm,  // TODO: get from swapchain
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR},
       // Depth attachment
       {vk::AttachmentDescriptionFlags(),
        vk::Format::eD32Sfloat,  // TODO: make configurables
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eDontCare,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal}}};

  vk::AttachmentReference color_reference {
      0, vk::ImageLayout::eColorAttachmentOptimal};

  vk::AttachmentReference depth_reference {
      1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

  // vk::SubpassDescription subpass_description {
  //     .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
  //     .colorAttachmentCount = 1,
  //     .pColorAttachments = &color_reference,
  //     .pDepthStencilAttachment = &depth_reference};

  vk::SubpassDescription subpassDescription(vk::SubpassDescriptionFlags(),
                                            vk::PipelineBindPoint::eGraphics,
                                            0,
                                            nullptr,
                                            1,
                                            &color_reference,
                                            nullptr,
                                            &depth_reference,
                                            0,
                                            nullptr);

  // Subpass dependencies for layout transitions
  // std::array<vk::SubpassDependency, 2> dependencies {{
  //     {.srcSubpass = vk::SubpassExternal,
  //      .dstSubpass = 0,
  //      .srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe,
  //      .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
  //          | vk::PipelineStageFlagBits::eEarlyFragmentTests
  //          | vk::PipelineStageFlagBits::eLateFragmentTests,
  //      .srcAccessMask = vk::AccessFlagBits::eNoneKHR,
  //      .dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead
  //          | vk::AccessFlagBits::eColorAttachmentWrite
  //          | vk::AccessFlagBits::eDepthStencilAttachmentRead
  //          | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
  //      .dependencyFlags = vk::DependencyFlagBits::eByRegion},
  //     {.srcSubpass = 0,
  //      .dstSubpass = vk::SubpassExternal,
  //      .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
  //          | vk::PipelineStageFlagBits::eEarlyFragmentTests
  //          | vk::PipelineStageFlagBits::eLateFragmentTests,
  //      .dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe,
  //      .srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead
  //          | vk::AccessFlagBits::eColorAttachmentWrite
  //          | vk::AccessFlagBits::eDepthStencilAttachmentRead
  //          | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
  //      .dstAccessMask = vk::AccessFlagBits::eMemoryRead,
  //      .dependencyFlags = vk::DependencyFlagBits::eByRegion},
  // }};

  std::array<vk::SubpassDependency, 2> dependencies {{
      {vk::SubpassExternal,
       0,
       vk::PipelineStageFlagBits::eBottomOfPipe,
       vk::PipelineStageFlagBits::eColorAttachmentOutput
           | vk::PipelineStageFlagBits::eEarlyFragmentTests
           | vk::PipelineStageFlagBits::eLateFragmentTests,
       vk::AccessFlagBits::eNoneKHR,
       vk::AccessFlagBits::eColorAttachmentRead
           | vk::AccessFlagBits::eColorAttachmentWrite
           | vk::AccessFlagBits::eDepthStencilAttachmentRead
           | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
       vk::DependencyFlagBits::eByRegion},
      {0,
       vk::SubpassExternal,
       vk::PipelineStageFlagBits::eColorAttachmentOutput
           | vk::PipelineStageFlagBits::eEarlyFragmentTests
           | vk::PipelineStageFlagBits::eLateFragmentTests,
       vk::PipelineStageFlagBits::eBottomOfPipe,
       vk::AccessFlagBits::eColorAttachmentRead
           | vk::AccessFlagBits::eColorAttachmentWrite
           | vk::AccessFlagBits::eDepthStencilAttachmentRead
           | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
       vk::AccessFlagBits::eMemoryRead,
       vk::DependencyFlagBits::eByRegion},
  }};

  // vk::RenderPassCreateInfo render_pass_create_info {
  //     .attachmentCount = static_cast<uint32_t>(attachments.size()),
  //     .pAttachments = attachments.data(),
  //     .subpassCount = 1,
  //     .pSubpasses = &subpassDescription,
  //     .dependencyCount = static_cast<uint32_t>(dependencies.size()),
  //     .pDependencies = dependencies.data()};

  vk::RenderPassCreateInfo renderPassCreateInfo(
      vk::RenderPassCreateFlags(),
      static_cast<uint32_t>(attachments.size()),
      attachments.data(),
      1,
      &subpassDescription,
      static_cast<uint32_t>(dependencies.size()),
      dependencies.data());

  return handle.createRenderPass(renderPassCreateInfo);
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
