#pragma once

#include <cstdint>
#include <unordered_map>

#include <vulkan/vulkan.hpp>

// #include "buffers/buffer.hpp"

class Compute
{
public:
  Compute(vk::Device& device);
  ~Compute();

  vk::CommandBuffer commandBuffer;  // Command buffer storing the
                                    // dispatch commands and barriers
  vk::CommandPool commandPool;  // Use a separate command pool (queue family
                                // may differ from the one used for graphics)
  vk::DescriptorSet descriptorSet;  // Compute shader bindings
  vk::DescriptorSetLayout descriptorSetLayout;  // Compute shader binding layout
  std::unordered_map<std::string, vk::Pipeline> pipelines;
  vk::PipelineLayout pipelineLayout;  // Layout of the compute pipeline
  vk::Queue queue;  // Separate queue for compute commands (queue family may
                    // differ from the one used for graphics)
  uint32_t queueFamilyIndex = 0;
  // vk::Semaphore
  //     semaphore;  // Execution dependency between compute & graphic
  //     submission
  uint32_t sharedDataSize = 1024;
  // std::unique_ptr<Buffer> storageBuffer;  // (Shader) storage buffer object
  //                                         // containing the particles
  // //   ComputeUBO ubo;
  // std::unique_ptr<Buffer> uniformBuffer;  // Uniform buffer object containing
  //                                         // particle system parameters
  uint32_t work_group_size = 128;

private:
  void destroy();
  vk::Device& device;
};
