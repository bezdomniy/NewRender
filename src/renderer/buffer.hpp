#pragma once

#include <cstddef>
#include <cstdint>

#include <vulkan/vulkan.hpp>

class Buffer
{
public:
  Buffer(vk::Device& device,
         vk::PhysicalDeviceMemoryProperties memoryProperties,
         vk::BufferUsageFlags usageFlags,
         vk::MemoryPropertyFlags memoryFlags,
         size_t size,
         void* data = nullptr);
  ~Buffer();

  vk::Buffer handle {VK_NULL_HANDLE};
  vk::DeviceMemory memory {VK_NULL_HANDLE};

private:
  vk::Device& device;

  uint32_t findMemoryType(
      vk::PhysicalDeviceMemoryProperties const& memoryProperties,
      uint32_t typeBits,
      vk::MemoryPropertyFlags requirementsMask);
};
