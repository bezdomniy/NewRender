
#include <iostream>
#include <stdexcept>

#include "hostBuffer.hpp"

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "vk_mem_alloc.h"

HostBuffer::HostBuffer(vk::Device& device,
                       VmaAllocator& allocator,
                       size_t size,
                       void* data,
                       vk::BufferUsageFlags usageFlags,
                       VmaMemoryUsage memoryUsage,
                       VmaAllocationCreateFlags flags)
    : Buffer(device, allocator)
{
  try {
    auto bufferCreateInfo = vk::BufferCreateInfo(
        vk::BufferCreateFlags(), size, usageFlags, vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = memoryUsage;
    allocCreateInfo.flags = flags;

    auto rawBufferCreateInfo =
        static_cast<VkBufferCreateInfo>(bufferCreateInfo);
    auto rawHandle = static_cast<VkBuffer>(handle);

    VkResult result = vmaCreateBuffer(allocator,
                                      &rawBufferCreateInfo,
                                      &allocCreateInfo,
                                      &rawHandle,
                                      &allocation,
                                      &allocInfo);

    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to create host buffer.");
    }

    memcpy(allocInfo.pMappedData, data, size);

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
