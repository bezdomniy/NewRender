#include "compute.hpp"

#include "device.hpp"
#include "executor.hpp"

Compute::Compute(
    vk::Device& device,
    uint32_t queueFamilyIndex,
    vk::DescriptorPool& descriptorPool,
    std::vector<vk::DescriptorSetLayoutBinding>& descriptorSetLayoutBindings)
    : Executor(
          device, queueFamilyIndex, descriptorPool, descriptorSetLayoutBindings)
{
}

Compute::~Compute() = default;
