#include <thread>

#include "buffers/deviceBuffer.hpp"
#include "buffers/hostBuffer.hpp"
#include "graphics.hpp"
#define VMA_IMPLEMENTATION

#include <cstdint>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

#include <fmt/base.h>
#include <fmt/ranges.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "renderer.hpp"
#include "shader.hpp"
#include "validation.hpp"

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

Renderer::Renderer(std::string name, Window* window, Game& game)
    : appName(std::move(name))
    , window(window)
    , game(game)
    , allocator(nullptr) {};

Renderer::~Renderer()
{
  cleanup();
}

[[noreturn]] void Renderer::run()
{
  initVulkan();
  initCompute();
  initGraphics();

  while (true) {
    update();
    // draw();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

HostBuffer& Renderer::createHostBuffer(const std::string& name,
                                       size_t size,
                                       void* data,
                                       vk::BufferUsageFlags usageFlags,
                                       VmaMemoryUsage memoryUsage,
                                       VmaAllocationCreateFlags flags)
{
  auto [fst, snd] = hostBuffers.try_emplace(name,
                                            device->handle,
                                            allocator,
                                            size,
                                            data,
                                            usageFlags,
                                            memoryUsage,
                                            flags);
  return fst->second;
};

DeviceBuffer& Renderer::createDeviceBuffer(const std::string& name,
                                           size_t size,
                                           vk::BufferUsageFlags usageFlags,
                                           VmaMemoryUsage memoryUsage,
                                           VmaAllocationCreateFlags flags)
{
  auto [fst, snd] = deviceBuffers.try_emplace(
      name, device->handle, allocator, size, usageFlags, memoryUsage, flags);
  return fst->second;
};

void Renderer::initVulkan()
{
  createInstance();
  surface = window->create_surface(instance);

  // device = std::make_unique<Device>(instance);
  device = std::make_unique<Device>(instance, &surface);

  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
  allocatorInfo.physicalDevice = device->physicalDevice;
  allocatorInfo.device = device->handle;
  allocatorInfo.instance = instance;

  vmaCreateAllocator(&allocatorInfo, &allocator);

  swapchain = device->createSwapchain();
  images = device->getSwapchainImages(swapchain);
  imagesViews = device->getImageViews(images);
}

void Renderer::initCompute()
{
  std::vector<vk::DescriptorPoolSize> computeDescriptorPoolSizes = {
      {vk::DescriptorPoolSize {vk::DescriptorType::eUniformBuffer, 1},
       vk::DescriptorPoolSize {vk::DescriptorType::eStorageBuffer, 2},
       vk::DescriptorPoolSize {vk::DescriptorType::eCombinedImageSampler, 1}}};

  const auto computeDescriptorPool =
      device->createDescriptorPool(computeDescriptorPoolSizes, 1);
  descriptorPools["compute"] = computeDescriptorPool;

  // Create compute executor
  std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
      vk::DescriptorSetLayoutBinding {0,
                                      vk::DescriptorType::eStorageBuffer,
                                      1,
                                      vk::ShaderStageFlagBits::eCompute},
      vk::DescriptorSetLayoutBinding {1,
                                      vk::DescriptorType::eStorageBuffer,
                                      1,
                                      vk::ShaderStageFlagBits::eCompute}};

  compute = std::make_unique<Compute>(
      device->handle,
      device->queueFamilyIndices.computeFamily.value(),
      descriptorPools["compute"],
      descriptorSetLayoutBindings);

  // Create and load buffers
  const auto resultSize = game.vertices.size();

  const HostBuffer& hostBuffer0 =
      createHostBuffer("buffer0",
                       game.vertices.size() * sizeof(glm::vec2),
                       game.vertices.data(),
                       vk::BufferUsageFlagBits::eTransferSrc);

  createHostBuffer("result",
                   game.vertices.size() * sizeof(glm::vec2),
                   game.vertices.data(),
                   vk::BufferUsageFlagBits::eTransferDst);

  const DeviceBuffer& deviceBuffer0 =
      createDeviceBuffer("buffer0",
                         game.vertices.size() * sizeof(glm::vec2),
                         vk::BufferUsageFlagBits::eStorageBuffer
                             | vk::BufferUsageFlagBits::eTransferDst);

  const DeviceBuffer& deviceResultBuffer =
      createDeviceBuffer("result",
                         resultSize * sizeof(glm::vec2),
                         vk::BufferUsageFlagBits::eStorageBuffer
                             | vk::BufferUsageFlagBits::eTransferSrc
                             | vk::BufferUsageFlagBits::eTransferDst);

  compute->commandBuffer.begin(vk::CommandBufferBeginInfo());

  compute->commandBuffer.copyBuffer(
      hostBuffer0.handle,
      deviceBuffer0.handle,
      {{0, 0, game.vertices.size() * sizeof(glm::vec2)}});

  vk::MemoryBarrier memoryBarrier;
  memoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
  memoryBarrier.dstAccessMask =
      vk::AccessFlagBits::eShaderRead;  // Access by compute shader

  compute->commandBuffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eTransfer,  // Src stage mask
      vk::PipelineStageFlagBits::eComputeShader,  // Dst stage mask
      {},  // Dependency flags
      memoryBarrier,  // Memory barriers
      nullptr,  // Buffer memory barriers (could use instead of memory barrier)
      nullptr);  // Image memory barriers

  // No barrier because using the same queue for now
  compute->commandBuffer.end();

  const vk::SubmitInfo submitInfo({}, {}, compute->commandBuffer);
  compute->queue.submit(submitInfo);
  compute->queue.waitIdle();

  // Create pipeline including descriptors and shaders
  vk::DescriptorBufferInfo buffer0Descriptor(
      deviceBuffer0.handle, 0, VK_WHOLE_SIZE);

  vk::DescriptorBufferInfo resultDescriptor(
      deviceResultBuffer.handle, 0, VK_WHOLE_SIZE);

  std::vector<vk::WriteDescriptorSet> writeDescriptorSets = {
      {{compute->descriptorSet,
        0,
        {},
        vk::DescriptorType::eStorageBuffer,
        {},
        buffer0Descriptor},
       {compute->descriptorSet,
        1,
        {},
        vk::DescriptorType::eStorageBuffer,
        {},
        resultDescriptor}}};

  device->handle.updateDescriptorSets(writeDescriptorSets, nullptr);

  std::string shaderPath = "src/shaders/bin/hello-world.slang.main.spv";
  const auto helloWorldShader =
      std::make_unique<Shader>(device.get(), shaderPath);
  auto stage = helloWorldShader->getShaderStageCreateInfo(
      vk::ShaderStageFlagBits::eCompute);

  compute->pipelines["compute1"] =
      device->createComputePipeline(stage, compute->pipelineLayout);
}

void Renderer::initGraphics()
{
  std::vector<vk::DescriptorPoolSize> graphicsDescriptorPoolSizes = {
      {vk::DescriptorPoolSize {vk::DescriptorType::eUniformBuffer, 2},
       vk::DescriptorPoolSize {vk::DescriptorType::eStorageBuffer, 3},
       vk::DescriptorPoolSize {vk::DescriptorType::eCombinedImageSampler, 2}}};

  auto graphicsDescriptorPool =
      device->createDescriptorPool(graphicsDescriptorPoolSizes, 1);
  descriptorPools["graphics"] = graphicsDescriptorPool;

  // Create graphics executor
  std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
      vk::DescriptorSetLayoutBinding {0,
                                      vk::DescriptorType::eCombinedImageSampler,
                                      1,
                                      vk::ShaderStageFlagBits::eFragment},
      vk::DescriptorSetLayoutBinding {1,
                                      vk::DescriptorType::eCombinedImageSampler,
                                      1,
                                      vk::ShaderStageFlagBits::eFragment},
      vk::DescriptorSetLayoutBinding {2,
                                      vk::DescriptorType::eUniformBuffer,
                                      1,
                                      vk::ShaderStageFlagBits::eVertex}};

  graphics = std::make_unique<Graphics>(
      device->handle,
      device->queueFamilyIndices.graphicsFamily.value(),
      descriptorPools["graphics"],
      descriptorSetLayoutBindings);

  std::string vertShaderPath = "src/shaders/bin/graphics.slang.vertMain.spv";
  const auto vertShader =
      std::make_unique<Shader>(device.get(), vertShaderPath);
  auto vertStage =
      vertShader->getShaderStageCreateInfo(vk::ShaderStageFlagBits::eVertex);

  std::string fragShaderPath = "src/shaders/bin/graphics.slang.fragMain.spv";
  const auto fragShader =
      std::make_unique<Shader>(device.get(), fragShaderPath);
  auto fragStage =
      fragShader->getShaderStageCreateInfo(vk::ShaderStageFlagBits::eFragment);

  vk::VertexInputBindingDescription vertexInputBindingDescription(
      0, sizeof(game.vertices.at(0)), vk::VertexInputRate::eVertex);
  std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttributes = {{
      {0,
       0,
       vk::Format::eR32G32B32A32Sfloat,
       offsetof(typeof(game.vertices.at(0)), pos)},  // Location 0 : Position
      //  {1,
      //   0,
      //   vk::Format::eR32G32B32A32Sfloat,
      //   offsetof(Particle, vel)}
  }};  // Location 1 : Velocity

  vk::PipelineVertexInputStateCreateInfo vertexInputBindingInfo(
      vk::PipelineVertexInputStateCreateFlags(),
      1,
      &vertexInputBindingDescription,
      static_cast<uint32_t>(vertexInputAttributes.size()),
      vertexInputAttributes.data());

  renderPass = device->createRenderPass();  // TODO should device own this??

  graphics->pipelines["graphics1"] =
      device->createGraphicsPipeline(vertStage,
                                     fragStage,
                                     vertexInputBindingInfo,
                                     renderPass,
                                     graphics->pipelineLayout);
}

void Renderer::update() const
{
  std::vector<glm::vec2> result {{0, 0}, {0, 0}, {0, 0}};

  const auto& hostResultBuffer = hostBuffers.at("result");
  const auto& deviceResultBuffer = deviceBuffers.at("result");

  // Execute compute pipeline
  compute->commandBuffer.begin(vk::CommandBufferBeginInfo());

  // Barrier to ensure that input buffer transfer is finished before compute
  // shader reads from it
  vk::BufferMemoryBarrier bufferBarrier {};
  bufferBarrier.buffer = deviceResultBuffer.handle;
  bufferBarrier.size = vk::WholeSize;
  bufferBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
  bufferBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
  bufferBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
  bufferBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

  compute->commandBuffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eHost,
      vk::PipelineStageFlagBits::eComputeShader,
      {},
      nullptr,
      bufferBarrier,
      nullptr);

  compute->commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute,
                                      compute->pipelines.at("compute1"));

  compute->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                            compute->pipelineLayout,
                                            0,
                                            compute->descriptorSet,
                                            {});

  compute->commandBuffer.dispatch(static_cast<uint32_t>(result.size()), 1, 1);

  // Barrier to ensure that shader writes are finished before buffer is read
  // back from GPU
  bufferBarrier.buffer = deviceResultBuffer.handle;
  bufferBarrier.size = vk::WholeSize;
  bufferBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
  bufferBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
  bufferBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
  bufferBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

  compute->commandBuffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eComputeShader,
      vk::PipelineStageFlagBits::eTransfer,
      {},
      nullptr,
      bufferBarrier,
      nullptr);

  // copy to host
  compute->commandBuffer.copyBuffer(
      deviceResultBuffer.handle,
      hostResultBuffer.handle,
      {{0, 0, result.size() * sizeof(glm::vec2)}});

  // Barrier to ensure that buffer copy is finished before host reading from it
  bufferBarrier.buffer = hostResultBuffer.handle;
  bufferBarrier.size = vk::WholeSize;
  bufferBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
  bufferBarrier.dstAccessMask = vk::AccessFlagBits::eHostRead;
  bufferBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
  bufferBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

  compute->commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                         vk::PipelineStageFlagBits::eHost,
                                         {},
                                         nullptr,
                                         bufferBarrier,
                                         nullptr);

  compute->commandBuffer.end();

  const auto fence = device->handle.createFence({});
  device->handle.resetFences(fence);

  const vk::SubmitInfo submitInfo({}, {}, compute->commandBuffer);
  compute->queue.submit(submitInfo, fence);

  if (device->handle.waitForFences(fence, vk::True, UINT64_MAX)
      != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to wait for fence.");
  }

  device->handle.destroyFence(fence);

  vmaFlushAllocation(allocator, hostResultBuffer.allocation, 0, vk::WholeSize);
  vmaInvalidateAllocation(
      allocator, hostResultBuffer.allocation, 0, vk::WholeSize);

  memcpy(result.data(),
         hostResultBuffer.allocInfo.pMappedData,
         result.size() * sizeof(glm::vec2));

  compute->queue.waitIdle();

  for (auto& item : result) {
    fmt::print("({},{}), ", item.x, item.y);
  }
  fmt::print("\n");
}

void Renderer::draw() const {}

void Renderer::cleanup()
{
  device->computeQueue.waitIdle();
  device->graphicsQueue.waitIdle();
  hostBuffers.clear();
  deviceBuffers.clear();

  vmaDestroyAllocator(allocator);

  compute.reset();
  graphics.reset();

  for (const auto& descriptorPool : descriptorPools | std::views::values) {
    device->handle.destroyDescriptorPool(descriptorPool);
  }

  device->handle.destroyRenderPass(renderPass);

  // device->handle.destroyCommandPool(commandPool);
  for (const auto& imageView : imagesViews) {
    device->handle.destroyImageView(imageView);
  }
  if (swapchain != VK_NULL_HANDLE) {
    device->handle.destroySwapchainKHR(swapchain);
  }
  device->destroy();
#if !defined(NDEBUG)
  instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
  vkDestroySurfaceKHR(instance, surface, nullptr);
  instance.destroy();
}

void Renderer::createInstance()
{
  try {
    uint32_t sdlExtensionCount = 0;
    const auto sdlExtensions = Window::getExtensions(&sdlExtensionCount);

    std::vector<const char*> enabledExtensions(
        sdlExtensions, sdlExtensions + sdlExtensionCount);

#if !defined(NDEBUG)
    enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    std::vector<const char*> enabledLayers;
#if !defined(NDEBUG)
    enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    const vk::ApplicationInfo applicationInfo(appName.c_str(),
                                              VK_MAKE_VERSION(0, 0, 1),
                                              "NewEngine",
                                              VK_MAKE_VERSION(0, 0, 1),
                                              VK_API_VERSION_1_3);

#ifdef __APPLE__
    auto flags = vk::InstanceCreateFlags {
        vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR};

    enabledExtensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    enabledExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#else
    auto flags = vk::InstanceCreateFlags {};
#endif

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
    // initialize with explicitly providing a DynamicLoader
    vk::detail::DynamicLoader dl;
    VULKAN_HPP_DEFAULT_DISPATCHER.init(dl);
#endif

    vk::InstanceCreateInfo instanceCreateInfo(
        flags,
        &applicationInfo,
        static_cast<uint32_t>(enabledLayers.size()),
        enabledLayers.data(),
        static_cast<uint32_t>(enabledExtensions.size()),
        enabledExtensions.data());

    instance = vk::createInstance(instanceCreateInfo);

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
    // initialize minimal set of function pointers
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
#endif

#if !defined(NDEBUG)
    debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(
        makeDebugUtilsMessengerCreateInfoEXT());
#endif

  } catch (vk::SystemError& err) {
    fmt::println("vk::SystemError: {}", err.what());
    exit(-1);
  }

  catch (std::exception& err)
  {
    fmt::println("std::exception: {}", err.what());
    exit(-1);
  }

  catch (...)
  {
    fmt::println("unknown error");
    exit(-1);
  }
}
