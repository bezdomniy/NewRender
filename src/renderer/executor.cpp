#include <ranges>

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
  vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo {
      .setLayoutCount = 1, .pSetLayouts = &descriptorSetLayout};
  pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

  device.getQueue(queueFamilyIndex, 0, &queue);

  commandPool = createCommandPool(
      {vk::CommandPoolCreateFlagBits::eResetCommandBuffer}, queueFamilyIndex);

  commandBuffer = allocateCommandBuffer(commandPool);
}

vk::DescriptorSetLayout Executor::createDescriptorSetLayout(
    std::vector<vk::DescriptorSetLayoutBinding>& bindings) const
{
  vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data()};

  return device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
}

vk::DescriptorSet Executor::allocateDescriptorSet(
    vk::DescriptorPool& descriptorPool,
    vk::DescriptorSetLayout& descriptorSetLayout) const
{
  vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo {
      .descriptorPool = descriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptorSetLayout};
  // TODO: why is this mutliple sets?
  return device.allocateDescriptorSets(descriptorSetAllocateInfo).front();
}

vk::CommandPool Executor::createCommandPool(vk::CommandPoolCreateFlags flags,
                                            uint32_t queueIndex) const
{
  vk::CommandPoolCreateInfo commandPoolCreateInfo {
      .flags = flags, .queueFamilyIndex = queueIndex};
  return device.createCommandPool(commandPoolCreateInfo);
}

vk::CommandBuffer Executor::allocateCommandBuffer(
    vk::CommandPool& commandPool, vk::CommandBufferLevel level) const
{
  vk::CommandBufferAllocateInfo commandBufferAllocateInfo {
      .commandPool = commandPool, .level = level, .commandBufferCount = 1};
  return device.allocateCommandBuffers(commandBufferAllocateInfo).front();
}

Executor::~Executor()
{
  destroy();
}

void Executor::destroy()
{
  // storageBuffer.reset();
  // uniformBuffer.reset();

  for (auto& val : pipelines | std::views::values) {
    device.destroyPipeline(val);
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
