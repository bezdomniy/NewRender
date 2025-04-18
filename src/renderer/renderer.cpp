#include "buffers/deviceBuffer.hpp"
#include "buffers/hostBuffer.hpp"
#define VMA_IMPLEMENTATION

#include <cstdint>
#include <memory>
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

Renderer::Renderer(std::string name, Window* window)
    : window(window)
    , appName(name) {};

Renderer::~Renderer()
{
  cleanup();
}

void Renderer::run()
{
  initVulkan();
  render();
  fmt::print("Hello {}!\n", appName);
}

HostBuffer& Renderer::createHostBuffer(std::string name,
                                       size_t size,
                                       void* data,
                                       vk::BufferUsageFlags usageFlags,
                                       VmaMemoryUsage memoryUsage,
                                       VmaAllocationCreateFlags flags)
{
  auto result = hostBuffers.try_emplace(name,
                                        device->handle,
                                        allocator,
                                        size,
                                        data,
                                        usageFlags,
                                        memoryUsage,
                                        flags);
  return result.first->second;
};

DeviceBuffer& Renderer::createDeviceBuffer(std::string name,
                                           size_t size,
                                           vk::BufferUsageFlags usageFlags,
                                           VmaMemoryUsage memoryUsage,
                                           VmaAllocationCreateFlags flags)
{
  auto result = deviceBuffers.try_emplace(
      name, device->handle, allocator, size, usageFlags, memoryUsage, flags);
  return result.first->second;
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
  imagesViews = device->getImageViews(swapchain, images);
}

void Renderer::render()
{
  compute = std::make_unique<Compute>(device->handle);
  compute->queue = device->computeQueue;
  std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
      vk::DescriptorSetLayoutBinding {0,
                                      vk::DescriptorType::eStorageBuffer,
                                      1,
                                      vk::ShaderStageFlagBits::eCompute},
      vk::DescriptorSetLayoutBinding {1,
                                      vk::DescriptorType::eStorageBuffer,
                                      1,
                                      vk::ShaderStageFlagBits::eCompute},
      vk::DescriptorSetLayoutBinding {2,
                                      vk::DescriptorType::eStorageBuffer,
                                      1,
                                      vk::ShaderStageFlagBits::eCompute}};

  compute->descriptorSetLayout =
      device->createDescriptorSetLayout(descriptorSetLayoutBindings);

  std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
      {vk::DescriptorPoolSize {vk::DescriptorType::eUniformBuffer, 2},
       vk::DescriptorPoolSize {vk::DescriptorType::eStorageBuffer, 3},
       vk::DescriptorPoolSize {vk::DescriptorType::eCombinedImageSampler, 2}}};

  descriptorPool = device->createDescriptorPool(descriptorPoolSizes, 1);

  compute->descriptorSet = device->allocateDescriptorSet(
      descriptorPool, compute->descriptorSetLayout);

  compute->pipelineLayout =
      device->handle.createPipelineLayout({{}, compute->descriptorSetLayout});

  compute->commandPool = device->createCommandPool(
      {vk::CommandPoolCreateFlagBits::eResetCommandBuffer},
      device->queueFamilyIndices.computeFamily.value());

  compute->commandBuffer = device->allocateCommandBuffer(compute->commandPool);

  std::vector<float> buffer0 {1, 2, 3};
  std::vector<float> buffer1 {2, 3, 4};
  std::vector<float> result {0, 99, 0};

  HostBuffer& hostBuffer0 =
      createHostBuffer("buffer0",
                       buffer0.size() * sizeof(float),
                       buffer0.data(),
                       vk::BufferUsageFlagBits::eTransferSrc);

  HostBuffer& hostBuffer1 =
      createHostBuffer("buffer1",
                       buffer1.size() * sizeof(float),
                       buffer1.data(),
                       vk::BufferUsageFlagBits::eTransferSrc);

  HostBuffer& hostResultBuffer =
      createHostBuffer("result",
                       buffer1.size() * sizeof(float),
                       buffer1.data(),
                       vk::BufferUsageFlagBits::eTransferDst);

  DeviceBuffer& deviceBuffer0 =
      createDeviceBuffer("buffer0",
                         buffer0.size() * sizeof(float),
                         vk::BufferUsageFlagBits::eStorageBuffer
                             | vk::BufferUsageFlagBits::eTransferDst);
  DeviceBuffer& deviceBuffer1 =
      createDeviceBuffer("buffer1",
                         buffer1.size() * sizeof(float),
                         vk::BufferUsageFlagBits::eStorageBuffer
                             | vk::BufferUsageFlagBits::eTransferDst);
  DeviceBuffer& deviceResultBuffer =
      createDeviceBuffer("result",
                         result.size() * sizeof(float),
                         vk::BufferUsageFlagBits::eStorageBuffer
                             | vk::BufferUsageFlagBits::eTransferSrc
                             | vk::BufferUsageFlagBits::eTransferDst);

  compute->commandBuffer.begin(vk::CommandBufferBeginInfo());

  compute->commandBuffer.copyBuffer(hostBuffer0.handle,
                                    deviceBuffer0.handle,
                                    {{0, 0, buffer0.size() * sizeof(float)}});
  compute->commandBuffer.copyBuffer(hostBuffer1.handle,
                                    deviceBuffer1.handle,
                                    {{0, 0, buffer1.size() * sizeof(float)}});

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

  vk::SubmitInfo si({}, {}, compute->commandBuffer);
  compute->queue.submit(si);
  compute->queue.waitIdle();

  vk::DescriptorBufferInfo buffer0Descriptor(
      deviceBuffer0.handle, 0, VK_WHOLE_SIZE);
  vk::DescriptorBufferInfo buffer1Descriptor(
      deviceBuffer1.handle, 0, VK_WHOLE_SIZE);
  vk::DescriptorBufferInfo resultDescriptor(
      deviceResultBuffer.handle, 0, VK_WHOLE_SIZE);

  std::vector<vk::WriteDescriptorSet> writeDescriptorSets = {
      {// Binding 0 : buffer0
       {compute->descriptorSet,
        0,
        {},
        vk::DescriptorType::eStorageBuffer,
        {},
        buffer0Descriptor},
       // Binding 1 : buffer1
       {compute->descriptorSet,
        1,
        {},
        vk::DescriptorType::eStorageBuffer,
        {},
        buffer1Descriptor},
       // Binding 1 : buffer1
       {compute->descriptorSet,
        2,
        {},
        vk::DescriptorType::eStorageBuffer,
        {},
        resultDescriptor}}};

  device->handle.updateDescriptorSets(writeDescriptorSets, nullptr);

  std::string shaderPath = "src/shaders/bin/hello-world.slang.spv";
  auto helloWorldShader = std::make_unique<Shader>(device.get(), shaderPath);
  auto stage = helloWorldShader->getShaderStageCreateInfo(
      vk::ShaderStageFlagBits::eCompute);

  auto fence = device->handle.createFence({});

  compute->pipelines["compute1"] =
      device->createComputePipeline(stage, compute->pipelineLayout);

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

  compute->commandBuffer.dispatch(result.size(), 1, 1);

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
  compute->commandBuffer.copyBuffer(deviceResultBuffer.handle,
                                    hostResultBuffer.handle,
                                    {{0, 0, result.size() * sizeof(float)}});

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

  device->handle.resetFences(fence);
  vk::SubmitInfo submitInfo({}, {}, compute->commandBuffer);
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
         result.size() * sizeof(float));

  compute->queue.waitIdle();

  fmt::print("Result: {}\n", fmt::join(result, ", "));
}

void Renderer::cleanup()
{
  device->computeQueue.waitIdle();
  device->graphicsQueue.waitIdle();
  hostBuffers.clear();
  deviceBuffers.clear();

  vmaDestroyAllocator(allocator);

  compute.reset();

  device->handle.destroyDescriptorPool(descriptorPool);

  // device->handle.destroyCommandPool(commandPool);
  for (auto& imageView : imagesViews) {
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
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = window->getExtensions(&glfwExtensionCount);

    std::vector<const char*> enabledExtensions(
        glfwExtensions, glfwExtensions + glfwExtensionCount);

#if !defined(NDEBUG)
    enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    std::vector<const char*> enabledLayers;
#if !defined(NDEBUG)
    enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    vk::ApplicationInfo applicationInfo(appName.c_str(),
                                        VK_MAKE_VERSION(0, 0, 1),
                                        "NewEngine",
                                        VK_MAKE_VERSION(0, 0, 1),
                                        VK_API_VERSION_1_3);

#ifdef __APPLE__
    vk::InstanceCreateFlags flags = vk::InstanceCreateFlags {
        vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR};

    enabledExtensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    enabledExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#else
    vk::InstanceCreateFlags flags = vk::InstanceCreateFlags {};
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
