#include "renderer.hpp"
#include "window.hpp"

int main()
{
  Window window("My Window", 1280, 720);
  Renderer renderer("My World", &window);
  renderer.run();

  return 0;
}
