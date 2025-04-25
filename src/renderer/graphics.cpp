#include "graphics.hpp"

Graphics::Graphics(
    vk::Device& device,
    uint32_t queueFamilyIndex,
    vk::DescriptorPool& descriptorPool,
    std::vector<vk::DescriptorSetLayoutBinding>& descriptorSetLayoutBindings)
    : Executor(
          device, queueFamilyIndex, descriptorPool, descriptorSetLayoutBindings)
{
}

Graphics::~Graphics() {}
