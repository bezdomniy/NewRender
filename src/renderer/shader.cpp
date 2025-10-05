// src/renderer/shader.cpp
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "shader.hpp"

#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "device.hpp"  // Include the Device class definition

Shader::Shader(Device* device, const std::string& shaderPath)
    : device(device)
{
  auto shaderCode = readFile(shaderPath);
  shaderModule = createShaderModule(shaderCode);
}

Shader::~Shader()
{
  if (shaderModule) {
    device->handle.destroyShaderModule(shaderModule);
  }
}

auto Shader::createShaderModule(const std::vector<char>& code)
    -> vk::ShaderModule
{
  vk::ShaderModuleCreateInfo createInfo;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

  try {
    return device->handle.createShaderModule(createInfo);
  } catch (vk::SystemError& err) {
    throw std::runtime_error("failed to create shader module!");
  }
}

vk::PipelineShaderStageCreateInfo Shader::getShaderStageCreateInfo(
    vk::ShaderStageFlagBits stage, const std::string& entryPoint)
{
  vk::PipelineShaderStageCreateInfo shaderStage;
  shaderStage.stage = stage;
  shaderStage.module = shaderModule;
  shaderStage.pName = entryPoint.c_str();
  return shaderStage;
}

auto Shader::readFile(const std::string& filePath) -> std::vector<char>
{
  std::ifstream file(filePath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error(std::string("failed to open file: ") + filePath);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
  file.close();

  return buffer;
}
