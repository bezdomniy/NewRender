#pragma once

#include <vulkan/vulkan.hpp>

#include "vk_mem_alloc.h"

class Buffer
{
public:
  Buffer(vk::Device& device, VmaAllocator& allocator);
  ~Buffer();

  vk::Buffer handle {VK_NULL_HANDLE};
  VmaAllocation allocation;
  VmaAllocationInfo allocInfo;

private:
  vk::Device& device;
  VmaAllocator& allocator;
};
