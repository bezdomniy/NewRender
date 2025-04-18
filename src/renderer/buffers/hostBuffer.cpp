
#include <iostream>

#include "hostBuffer.hpp"

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

    vmaCreateBuffer(allocator,
                    &rawBufferCreateInfo,
                    &allocCreateInfo,
                    &rawHandle,
                    &allocation,
                    &allocInfo);

    // uint8_t* pData = static_cast<uint8_t*>(
    //     device.mapMemory(memory, 0, memoryRequirements.size));
    memcpy(allocInfo.pMappedData, data, size);
    // device.unmapMemory(memory);

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
