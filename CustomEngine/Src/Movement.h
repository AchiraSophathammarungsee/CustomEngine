
#ifndef MOVEMENT_H
#define MOVEMENT_H
#include <glm/glm.hpp>

struct Movement
{
  glm::vec3 Velocity;
  glm::vec3 Acceleration;

  void DrawDebugMenu();
};

#endif