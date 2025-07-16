#include "renderer.hpp"
#include "window.hpp"

int main()
{
  Window window("My Window", 1280, 720);
  Game game;
  Renderer renderer("My World", &window, game);
  renderer.run();

  return 0;
}
