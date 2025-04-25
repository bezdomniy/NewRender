#pragma once

#include <cstdint>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "executor.hpp"

class Compute : public Executor
{
public:
  Compute(
      vk::Device& device,
      uint32_t queueFamilyIndex,
      vk::DescriptorPool& descriptorPool,
      std::vector<vk::DescriptorSetLayoutBinding>& descriptorSetLayoutBindings);
  ~Compute();

  Compute(const Compute&) = delete;  // Disable copy constructor
  Compute& operator=(const Compute&) = delete;  // Disable copy assignment
  Compute(Compute&&) = delete;  // Disable move constructor
  Compute& operator=(Compute&&) = delete;  // Disable move assignment
private:
};
