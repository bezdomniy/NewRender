#pragma once

#include <cstdint>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "executor.hpp"

class Graphics : public Executor
{
public:
  Graphics(
      vk::Device& device,
      uint32_t queueFamilyIndex,
      vk::DescriptorPool& descriptorPool,
      std::vector<vk::DescriptorSetLayoutBinding>& descriptorSetLayoutBindings);
  ~Graphics();

  Graphics(const Graphics&) = delete;  // Disable copy constructor
  Graphics& operator=(const Graphics&) = delete;  // Disable copy assignment
  Graphics(Graphics&&) = delete;  // Disable move constructor
  Graphics& operator=(Graphics&&) = delete;  // Disable move assignment
private:
};
