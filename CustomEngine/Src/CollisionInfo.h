#pragma once
#ifndef COLLISIONINFO_H
#define COLLISIONINFO_H

#include <glm/glm.hpp>

class Collider;

struct CollisionInfo
{
  CollisionInfo() : Hit(false), other(nullptr), point(glm::vec3()), normal(glm::vec3()) {};
  CollisionInfo(bool hit, const glm::vec3 &_point, const glm::vec3 &norm, const Collider* othr) : Hit(hit), other(othr), point(_point), normal(norm) {};

  bool Hit;
  const Collider* other;
  glm::vec3 point;
  glm::vec3 normal;
};

#endif