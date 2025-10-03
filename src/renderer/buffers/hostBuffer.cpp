
#include <iostream>
#include <stdexcept>

#include "hostBuffer.hpp"

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "vk_mem_alloc.h"

HostBuffer::HostBuffer(vk::Device& device,
                       VmaAllocator& allocator,
                       const size_t size,
                       const void* data,
                       const vk::BufferUsageFlags usageFlags,
                       const VmaMemoryUsage memoryUsage,
                       VmaAllocationCreateFlags flags)
    : Buffer(device, allocator)
{
  try {
    vk::BufferCreateInfo bufferCreateInfo {
        .size = size,
        .usage = usageFlags,
        .sharingMode = vk::SharingMode::eExclusive,
    };

    VmaAllocationCreateInfo allocCreateInfo {
        .flags = flags, .usage = memoryUsage, .priority = 1.0f};

    auto rawBufferCreateInfo =
        static_cast<VkBufferCreateInfo>(bufferCreateInfo);

    VkResult result = vmaCreateBuffer(allocator,
                                      &rawBufferCreateInfo,
                                      &allocCreateInfo,
                                      &handle,
                                      &allocation,
                                      &allocInfo);

    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to create host buffer.");
    }

    vmaCopyMemoryToAllocation(allocator, data, allocation, 0, size);

  } catch (vk::SystemError& err) {
    std::cout << "vk::SystemError: " << err.what() << std::endl;
    exit(-1);
  } catch (std::exception& err) {
    std::cout << "std::exception: " << err.what() << std::endl;
    exit(-1);
  } catch (...) {
    std::cout << "unknown error\n";
    exit(-1);
  }
};
