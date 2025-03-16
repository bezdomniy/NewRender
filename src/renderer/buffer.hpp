#pragma once

#include <cstddef>
#include <cstdint>

#include <vulkan/vulkan.hpp>

class Buffer
{
public:
  Buffer(vk::Device& device,
         vk::PhysicalDeviceMemoryProperties memoryProperties,
         void* data,
         size_t size,
         vk::BufferUsageFlags flags,
         vk::MemoryPropertyFlags memoryFlags);
  ~Buffer();

private:
  vk::DeviceMemory memory {VK_NULL_HANDLE};
  vk::Buffer handle {VK_NULL_HANDLE};
  vk::Device& device;

  uint32_t findMemoryType(
      vk::PhysicalDeviceMemoryProperties const& memoryProperties,
      uint32_t typeBits,
      vk::MemoryPropertyFlags requirementsMask);
};
