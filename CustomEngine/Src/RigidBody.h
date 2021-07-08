#pragma once
#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <glm/glm.hpp>

class RigidBody
{
public:
  float Mass;

  glm::vec3 Velocity;
  glm::vec3 Acceleration;
  glm::vec3 Force;
  
  float AngularVel;
  float AngularAcc;
  glm::vec3 Torque;
private:
  
};

#endif