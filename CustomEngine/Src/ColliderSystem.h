#pragma once
#ifndef COLLIDERSYSTEM_H
#define COLLIDERSYSTEM_H

#include "System.h"
#include <glm/glm.hpp>

class ColliderSystem : public System
{
public:
  ColliderSystem() {};

  std::vector<EntityID> &GetEntities() { return Entities; };
  virtual void Update() = 0;

  //virtual bool PointTest(const glm::vec3& point) = 0;

private:

};

class ColliderBoxSystem : public ColliderSystem
{
public:
  ColliderBoxSystem() {};

  void Update() override;
  //bool PointTest(const glm::vec3& point) override;

private:

};

class ColliderSphereSystem : public ColliderSystem
{
public:
  ColliderSphereSystem() {};

  void Update() override;
  //bool PointTest(const glm::vec3& point) override;

private:

};

#endif