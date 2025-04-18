
#include <iostream>

#include "deviceBuffer.hpp"

#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "vk_mem_alloc.h"

DeviceBuffer::DeviceBuffer(vk::Device& device,
                           VmaAllocator& allocator,
                           size_t size,
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
    allocCreateInfo.priority = 1.0f;

    auto rawBufferCreateInfo =
        static_cast<VkBufferCreateInfo>(bufferCreateInfo);
    auto rawHandle = static_cast<VkBuffer>(handle);

    vmaCreateBuffer(allocator,
                    &rawBufferCreateInfo,
                    &allocCreateInfo,
                    &rawHandle,
                    &allocation,
                    &allocInfo);

    // device.bindBufferMemory(handle, allocInfo.deviceMemory, 0);
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
