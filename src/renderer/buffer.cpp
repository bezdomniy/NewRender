#include <iostream>

#include "buffer.hpp"

#include <vulkan/vulkan_enums.hpp>

Buffer::Buffer(vk::Device& device,
               vk::PhysicalDeviceMemoryProperties memoryProperties,
               vk::BufferUsageFlags usageFlags,
               vk::MemoryPropertyFlags memoryFlags,
               size_t size,
               void* data)
    : device(device)
{
  try {
    auto bufferCreateInfo = vk::BufferCreateInfo(
        vk::BufferCreateFlags(), size, usageFlags, vk::SharingMode::eExclusive);

    handle = device.createBuffer(bufferCreateInfo);

    auto memoryRequirements = device.getBufferMemoryRequirements(handle);
    auto typeIndex = findMemoryType(
        memoryProperties, memoryRequirements.memoryTypeBits, memoryFlags);

    memory = device.allocateMemory(
        vk::MemoryAllocateInfo(memoryRequirements.size, typeIndex));

    if (data != nullptr) {
      uint8_t* pData = static_cast<uint8_t*>(
          device.mapMemory(memory, 0, memoryRequirements.size));
      memcpy(pData, data, size);
      device.unmapMemory(memory);
    }

    device.bindBufferMemory(handle, memory, 0);
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

Buffer::~Buffer()
{
  device.destroyBuffer(handle);
  device.freeMemory(memory);
};

uint32_t Buffer::findMemoryType(
    vk::PhysicalDeviceMemoryProperties const& memoryProperties,
    uint32_t typeBits,
    vk::MemoryPropertyFlags requirementsMask)
{
  uint32_t typeIndex = uint32_t(~0);
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((typeBits & 1)
        && ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask)
            == requirementsMask))
    {
      typeIndex = i;
      break;
    }
    typeBits >>= 1;
  }
  assert(typeIndex != uint32_t(~0));
  return typeIndex;
}
