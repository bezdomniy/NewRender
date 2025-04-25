#include "executor.hpp"

Executor::Executor(
    vk::Device& device,
    uint32_t queueFamilyIndex,
    vk::DescriptorPool& descriptorPool,
    std::vector<vk::DescriptorSetLayoutBinding>& descriptorSetLayoutBindings)
    : device(device)
{
  descriptorSetLayout = createDescriptorSetLayout(descriptorSetLayoutBindings);
  descriptorSet = allocateDescriptorSet(descriptorPool, descriptorSetLayout);
  pipelineLayout = device.createPipelineLayout({{}, descriptorSetLayout});

  device.getQueue(queueFamilyIndex, 0, &queue);

  commandPool = createCommandPool(
      {vk::CommandPoolCreateFlagBits::eResetCommandBuffer}, queueFamilyIndex);

  commandBuffer = allocateCommandBuffer(commandPool);
}

vk::DescriptorSetLayout Executor::createDescriptorSetLayout(
    std::vector<vk::DescriptorSetLayoutBinding>& bindings)
{
  return device.createDescriptorSetLayout({{}, bindings, {}});
}

vk::DescriptorSet Executor::allocateDescriptorSet(
    vk::DescriptorPool& descriptorPool,
    vk::DescriptorSetLayout& descriptorSetLayout)
{
#if defined(ANDROID)
  vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(
      descriptorPool, 1, &descriptorSetLayout);
#else
  vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(descriptorPool,
                                                          descriptorSetLayout);
#endif
  // TODO: why is this mutliple sets?
  return device.allocateDescriptorSets(descriptorSetAllocateInfo).front();
}

vk::CommandPool Executor::createCommandPool(vk::CommandPoolCreateFlags flags,
                                            uint32_t queueIndex)
{
  return device.createCommandPool({flags, queueIndex});
}

vk::CommandBuffer Executor::allocateCommandBuffer(vk::CommandPool& commandPool,
                                                  vk::CommandBufferLevel level)
{
  return device.allocateCommandBuffers({commandPool, level, 1}).front();
}

Executor::~Executor()
{
  destroy();
}

void Executor::destroy()
{
  // storageBuffer.reset();
  // uniformBuffer.reset();

  for (auto& pipeline : pipelines) {
    device.destroyPipeline(pipeline.second);
  }

  device.destroyPipelineLayout(pipelineLayout);
  // no need to free the descriptor_set, as it's implicitly free'd with the
  // descriptor_pool
  device.destroyDescriptorSetLayout(descriptorSetLayout);
  // device.destroySemaphore(semaphore);
  device.freeCommandBuffers(commandPool, commandBuffer);
  device.destroyCommandPool(commandPool);

  pipelines.clear();
}
