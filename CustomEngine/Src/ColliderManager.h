#pragma once
#ifndef COLLIDERMANAGER_H
#define COLLIDERMANAGER_H

#include <glm/glm.hpp>
#include <memory>
#include "CollisionInfo.h"
#include "Ray.h"

class ColliderBoxSystem;
class ColliderSphereSystem;

#define MAX_COLLISION_LAYER 100
typedef unsigned int CollisionLayer;

class ColliderManager
{
public:
  static void Init(std::shared_ptr<ColliderBoxSystem> boxsys, std::shared_ptr<ColliderSphereSystem> spheresys);
  static void Update();

  static CollisionInfo PointTest(const glm::vec3 &point);
  static CollisionInfo RayTest(const Ray &ray, float length);

private:
  ColliderManager() {};
  ~ColliderManager() {};
  
  static std::shared_ptr<ColliderBoxSystem> BoxSystem;
  static std::shared_ptr<ColliderSphereSystem> SphereSystem;
};

#endif