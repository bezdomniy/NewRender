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

Graphics::~Graphics() = default;

void Graphics::insertImageMemoryBarrier(
    vk::Image image,
    vk::AccessFlags srcAccessMask,
    vk::AccessFlags dstAccessMask,
    vk::ImageLayout oldImageLayout,
    vk::ImageLayout newImageLayout,
    vk::PipelineStageFlags srcStageMask,
    vk::PipelineStageFlags dstStageMask,
    vk::ImageSubresourceRange subresourceRange) const
{
  vk::ImageMemoryBarrier imageMemoryBarrier;
  imageMemoryBarrier.srcAccessMask = srcAccessMask;
  imageMemoryBarrier.dstAccessMask = dstAccessMask;
  imageMemoryBarrier.oldLayout = oldImageLayout;
  imageMemoryBarrier.newLayout = newImageLayout;
  imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.image = image;
  imageMemoryBarrier.subresourceRange = subresourceRange;

  commandBuffer.pipelineBarrier(
      srcStageMask, dstStageMask, {}, nullptr, nullptr, {imageMemoryBarrier});
}
