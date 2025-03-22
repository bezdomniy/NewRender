#include <cstdint>
#include <memory>
#include <vector>

#include "renderer.hpp"

#include <fmt/base.h>
#include <fmt/ranges.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

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

void Renderer::createBuffer(std::string name,
                            vk::BufferUsageFlags usageFlags,
                            vk::MemoryPropertyFlags memoryFlags,
                            size_t size,
                            void* data)
{
  // TODO: copy from staging buffer to storage buffer
  buffers.try_emplace(name,
                      device->handle,
                      device->memoryProperties,
                      usageFlags,
                      memoryFlags,
                      size,
                      data);
};

Buffer Renderer::createBuffer(vk::BufferUsageFlags usageFlags,
                              vk::MemoryPropertyFlags memoryFlags,
                              size_t size,
                              void* data)
{
  return Buffer(device->handle,
                device->memoryProperties,
                usageFlags,
                memoryFlags,
                size,
                data);
};

void Renderer::initVulkan()
{
  createInstance();
  surface = window->create_surface(instance);

  // device = std::make_unique<Device>(instance);
  device = std::make_unique<Device>(instance, &surface);

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
  std::vector<float> result {0, 0, 0};
  createBuffer("buffer0",
               vk::BufferUsageFlagBits::eStorageBuffer,
               vk::MemoryPropertyFlagBits::eHostVisible
                   | vk::MemoryPropertyFlagBits::eHostCoherent,
               buffer0.size() * sizeof(float),
               buffer0.data());
  createBuffer("buffer1",
               vk::BufferUsageFlagBits::eStorageBuffer,
               vk::MemoryPropertyFlagBits::eHostVisible
                   | vk::MemoryPropertyFlagBits::eHostCoherent,
               buffer1.size() * sizeof(float),
               buffer1.data());
  auto resultStaging = createBuffer(vk::BufferUsageFlagBits::eTransferSrc
                                        | vk::BufferUsageFlagBits::eTransferDst,
                                    vk::MemoryPropertyFlagBits::eHostVisible,
                                    result.size() * sizeof(float),
                                    result.data());

  createBuffer("result",
               vk::BufferUsageFlagBits::eStorageBuffer
                   | vk::BufferUsageFlagBits::eTransferSrc
                   | vk::BufferUsageFlagBits::eTransferDst,
               vk::MemoryPropertyFlagBits::eDeviceLocal,
               result.size() * sizeof(float));

  compute->commandBuffer.begin(vk::CommandBufferBeginInfo());
  compute->commandBuffer.copyBuffer(resultStaging.handle,
                                    buffers.at("result").handle,
                                    {{0, 0, result.size() * sizeof(float)}});
  // No barrier because using the same queue for now
  compute->commandBuffer.end();
  vk::SubmitInfo si({}, {}, compute->commandBuffer);
  compute->queue.submit(si);
  compute->queue.waitIdle();

  vk::DescriptorBufferInfo buffer0Descriptor(
      buffers.at("buffer0").handle, 0, VK_WHOLE_SIZE);
  vk::DescriptorBufferInfo buffer1Descriptor(
      buffers.at("buffer1").handle, 0, VK_WHOLE_SIZE);
  vk::DescriptorBufferInfo resultDescriptor(
      buffers.at("result").handle, 0, VK_WHOLE_SIZE);

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

  compute->pipelines["compute1"] =
      device->createComputePipeline(stage, compute->pipelineLayout);

  compute->commandBuffer.begin(vk::CommandBufferBeginInfo());

  // Barrier to ensure that input buffer transfer is finished before compute
  // shader reads from it
  vk::BufferMemoryBarrier bufferBarrier {};
  bufferBarrier.buffer = buffers.at("result").handle;
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

  compute->commandBuffer.dispatch(1, 1, 1);

  // Barrier to ensure that shader writes are finished before buffer is read
  // back from GPU
  bufferBarrier.buffer = buffers.at("result").handle;
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
  compute->commandBuffer.copyBuffer(buffers.at("result").handle,
                                    resultStaging.handle,
                                    {{0, 0, result.size() * sizeof(float)}});

  // Barrier to ensure that buffer copy is finished before host reading from it
  bufferBarrier.buffer = resultStaging.handle;
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

  vk::SubmitInfo submitInfo({}, {}, compute->commandBuffer);
  compute->queue.submit(submitInfo);

  // Make device writes visible to the host
  void* mapped =
      device->handle.mapMemory(resultStaging.memory, 0, vk::WholeSize);

  vk::MappedMemoryRange mappedRange(resultStaging.memory);
  device->handle.flushMappedMemoryRanges(mappedRange);
  device->handle.invalidateMappedMemoryRanges({{mappedRange}});

  memcpy(result.data(), mapped, result.size() * sizeof(float));

  device->handle.unmapMemory(resultStaging.memory);
  compute->queue.waitIdle();

  fmt::print("Result: {}\n", fmt::join(result, ", "));
}

void Renderer::cleanup()
{
  device->computeQueue.waitIdle();
  device->graphicsQueue.waitIdle();
  buffers.clear();

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
                                        VK_API_VERSION_1_4);

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
