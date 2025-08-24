#pragma once
#include <vector>

#include <glm/glm.hpp>

struct GameVertex
{
  glm::vec2 pos;
};

class Game
{
public:
  Game();
  ~Game() = default;

  // temporary
  std::vector<GameVertex> vertices;

private:
};
