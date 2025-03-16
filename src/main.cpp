#include "renderer.hpp"
#include "window.hpp"

int main()
{
  Window window("My Window", 1280, 720);
  Renderer renderer("My World", &window);
  renderer.run();

  std::vector<uint32_t> numbers {1, 2, 3};
  renderer.createBuffer("numbers",
                 numbers.data(),
                 numbers.size() * sizeof(uint32_t),
                 vk::BufferUsageFlagBits::eUniformBuffer,
                 vk::MemoryPropertyFlagBits::eHostVisible
                     | vk::MemoryPropertyFlagBits::eHostCoherent);
  return 0;
}
