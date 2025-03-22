// src/renderer/shader.cpp
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "shader.hpp"

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

vk::ShaderModule Shader::createShaderModule(const std::vector<char>& code)
{
  vk::ShaderModuleCreateInfo createInfo;
  createInfo.codeSize = code.size();
  createInfo.pCode =
      reinterpret_cast<const uint32_t*>(code.data());  // Important cast

  try {
    return device->handle.createShaderModule(createInfo);
  } catch (vk::SystemError& err) {
    throw std::runtime_error("failed to create shader module!");
  }
}

std::vector<char> Shader::readFile(const std::string& filePath)
{
  std::ifstream file(filePath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error(std::string("failed to open file: ") + filePath);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}
