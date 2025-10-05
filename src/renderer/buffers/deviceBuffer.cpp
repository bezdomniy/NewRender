
#include <iostream>

#include "deviceBuffer.hpp"

#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

DeviceBuffer::DeviceBuffer(vk::Device& device,
                           VmaAllocator& allocator,
                           size_t size,
                           vk::BufferUsageFlags usageFlags,
                           VmaMemoryUsage memoryUsage,
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
        .flags = flags, .usage = memoryUsage, .priority = 1.0F};

    auto rawBufferCreateInfo =
        static_cast<VkBufferCreateInfo>(bufferCreateInfo);

    vmaCreateBuffer(allocator,
                    &rawBufferCreateInfo,
                    &allocCreateInfo,
                    &handle,
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
