#include "buffer.hpp"

#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "vk_mem_alloc.h"

Buffer::Buffer(vk::Device& device, VmaAllocator& allocator)
    : device(device)
    , allocator(allocator) {};

Buffer::~Buffer()
{
  auto rawHandle = static_cast<VkBuffer>(handle);
  vmaDestroyBuffer(allocator, rawHandle, allocation);
};
