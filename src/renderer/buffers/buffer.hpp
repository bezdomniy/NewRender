#pragma once

#include <vulkan/vulkan.hpp>

#include "vk_mem_alloc.h"

class Buffer
{
public:
  Buffer(vk::Device& device, VmaAllocator& allocator);
  virtual ~Buffer();

  Buffer(const Buffer&) = delete;  // Disable copy constructor
  Buffer& operator=(const Buffer&) = delete;  // Disable copy assignment
  Buffer(Buffer&&) = delete;  // Disable move constructor
  Buffer& operator=(Buffer&&) = delete;  // Disable move assignment

  VkBuffer handle {VK_NULL_HANDLE};
  VmaAllocation allocation = VK_NULL_HANDLE;
  VmaAllocationInfo allocInfo = {};

  auto getHandle() -> vk::Buffer;

protected:
  vk::Device& device;
  VmaAllocator& allocator;
};
