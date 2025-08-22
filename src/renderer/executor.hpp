#pragma once

#include <unordered_map>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

class Executor
{
public:
  Executor(
      vk::Device& device,
      uint32_t queueFamilyIndex,
      vk::DescriptorPool& descriptorPool,
      std::vector<vk::DescriptorSetLayoutBinding>& descriptorSetLayoutBindings);
  ~Executor();

  Executor(const Executor&) = delete;  // Disable copy constructor
  Executor& operator=(const Executor&) = delete;  // Disable copy assignment
  Executor(Executor&&) = delete;  // Disable move constructor
  Executor& operator=(Executor&&) = delete;  // Disable move assignment

  vk::CommandBuffer commandBuffer;  // Command buffer storing the
                                    // dispatch commands and barriers
  vk::CommandPool commandPool;  // Use a separate command pool (queue family
                                // may differ from the one used for graphics)
  vk::DescriptorSet descriptorSet;  // shader bindings
  vk::DescriptorSetLayout descriptorSetLayout;  // shader binding layout
  std::unordered_map<std::string, vk::Pipeline> pipelines;
  vk::PipelineLayout pipelineLayout;  // Layout of the pipeline
  vk::Queue queue;  // Separate queue for commands (queue family may
                    // differ from the one used for graphics)
  uint32_t queueFamilyIndex = 0;

protected:
  void destroy();

private:
  vk::DescriptorSet allocateDescriptorSet(
      vk::DescriptorPool& descriptorPool,
      vk::DescriptorSetLayout& descriptorSetLayout) const;

  vk::DescriptorSetLayout createDescriptorSetLayout(
      std::vector<vk::DescriptorSetLayoutBinding>& bindings) const;
  [[nodiscard]] vk::CommandPool createCommandPool(
      vk::CommandPoolCreateFlags flags, uint32_t queueIndex) const;
  vk::CommandBuffer allocateCommandBuffer(
      vk::CommandPool& commandPool,
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;
  vk::Device& device;
};
