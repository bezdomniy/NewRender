#include "buffer.hpp"

#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "vk_mem_alloc.h"

Buffer::Buffer(vk::Device& device, VmaAllocator& allocator)
    : device(device)
    , allocator(allocator) {};

Buffer::~Buffer()
{
  vmaDestroyBuffer(allocator, handle, allocation);
  handle = VK_NULL_HANDLE;
  allocation = VK_NULL_HANDLE;
};
