#pragma once

#include <cstddef>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

#include "buffer.hpp"
#include "vk_mem_alloc.h"

class HostBuffer : public virtual Buffer
{
public:
  HostBuffer(
      vk::Device& device,
      VmaAllocator& allocator,
      size_t size,
      void* data,
      vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferSrc
          | vk::BufferUsageFlagBits::eTransferDst,
      VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO,
      VmaAllocationCreateFlags flags =
          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
          | VMA_ALLOCATION_CREATE_MAPPED_BIT);

  HostBuffer(const HostBuffer&) = delete;
  HostBuffer& operator=(const HostBuffer&) = delete;
  HostBuffer(HostBuffer&&) = delete;
  HostBuffer& operator=(HostBuffer&&) = delete;

private:
};
