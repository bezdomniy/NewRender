#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

class Device;

class Shader
{
public:
  Shader(Device* device, const std::string& shaderPath);
  ~Shader();

  vk::ShaderModule getShaderModule() const { return shaderModule; }

private:
  Device* device;
  vk::ShaderModule shaderModule {VK_NULL_HANDLE};

  vk::ShaderModule createShaderModule(const std::vector<char>& code);
  std::vector<char> readFile(const std::string& filePath);
};
