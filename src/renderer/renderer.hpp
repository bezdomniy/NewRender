#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "../application/game.hpp"
#include "../application/window.hpp"
#include "buffers/buffer.hpp"
#include "buffers/deviceBuffer.hpp"
#include "buffers/hostBuffer.hpp"
#include "compute.hpp"
#include "device.hpp"
#include "graphics.hpp"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

class Renderer
{
public:
  Renderer(std::string name, Window* window, Game& game);
  ~Renderer();

  HostBuffer& createHostBuffer(
      const std::string& name,
      size_t size = 0,
      void* data = nullptr,
      vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferSrc
          | vk::BufferUsageFlagBits::eTransferDst,
      VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO,
      VmaAllocationCreateFlags flags =
          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
          | VMA_ALLOCATION_CREATE_MAPPED_BIT);

  DeviceBuffer& createDeviceBuffer(
      const std::string& name,
      size_t size = 0,
      vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
      VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO,
      VmaAllocationCreateFlags flags =
          VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

  // void createComputeTask(std::string name);
  [[noreturn]] void run();

private:
  std::string appName;

  vk::Instance instance {VK_NULL_HANDLE};

  Window* window = nullptr;
  Game& game;
  vk::SurfaceKHR surface {VK_NULL_HANDLE};
  std::unordered_map<std::string, vk::DescriptorPool> descriptorPools;
  // vk::CommandPool commandPool {VK_NULL_HANDLE};
  // std::vector<vk::CommandBuffer> commandBuffers;

  VmaAllocator allocator;

  vk::SwapchainKHR swapchain {VK_NULL_HANDLE};
  vk::Extent2D swapchainExtent;
  uint32_t currentImageIndex {0};
  // uint32_t currentBuffer {0};  // TODO: not used yet
  std::vector<vk::Image> images;
  std::vector<vk::ImageView> imagesViews;
  std::unordered_map<std::string, DeviceBuffer> deviceBuffers;
  std::unordered_map<std::string, HostBuffer> hostBuffers;

  std::vector<vk::Semaphore> recycledSemaphores;

  std::unique_ptr<Device> device = nullptr;
  std::unique_ptr<Compute> compute = nullptr;
  std::unique_ptr<Graphics> graphics = nullptr;

#if !defined(NDEBUG)
  vk::DebugUtilsMessengerEXT debugUtilsMessenger {VK_NULL_HANDLE};
#endif

  void initVulkan();
  void initCompute();
  void initGraphics();
  void update() const;
  void draw();
  void cleanup();

  void createInstance();
};
