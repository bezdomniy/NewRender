#pragma once

#include <cstddef>

#include <vulkan/vulkan.hpp>

#include "buffer.hpp"
#include "vk_mem_alloc.h"

class DeviceBuffer : public virtual Buffer
{
public:
  DeviceBuffer(
      vk::Device& device,
      VmaAllocator& allocator,
      size_t size,
      vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
      VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO,
      VmaAllocationCreateFlags flags =
          VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

private:
};
