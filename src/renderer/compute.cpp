#include "compute.hpp"

#include "device.hpp"

Compute::Compute(vk::Device& device)
    : device(device)
{
}

Compute::~Compute()
{
  destroy();
}

void Compute::destroy()
{
  storageBuffer.reset();
  uniformBuffer.reset();

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
