#include "game.hpp"

Game::Game()
{
  vertices = {{{0.0, -0.5}}, {{0.5, 0.5}}, {{-0.5, 0.5}}};

  // vertices = {
  //     {{-0.5f, -0.5f}},  // Bottom-left
  //     {{0.5f, -0.5f}},  // Bottom-right
  //     {{0.5f, 0.5f}},  // Top-right
  //     {{-0.5f, 0.5f}}  // Top-left
  // };
}
