#pragma once
#ifndef VQS_H
#define VQS_H

#include <glm/glm.hpp>
#include "Quaternion.h"

class VQS
{
public:
  VQS(const glm::vec3 &position = glm::vec3(), const glm::vec3 &scale = glm::vec3(1.f), const Quaternion &rotation = Quaternion());

  void UpdateMatrix();
  const glm::mat4& GetMatrix() const;

  const glm::vec3& GetPosition() const { return Position; };
  const glm::vec3& GetScale() const { return Scale; };
  const Quaternion& GetRotation() const { return Rotation; };

  void SetPosition(const glm::vec3& pos) { Position = pos; NeedUpdate = true; };
  void SetScale(const glm::vec3 &scale) { Scale = scale; NeedUpdate = true; };
  void SetRotation(const Quaternion& rot) { Rotation = rot; NeedUpdate = true; };

  glm::vec4 operator*(const glm::vec4& vector) const;
  VQS operator*(const VQS& other) const;
  VQS Inverse() const;

  static VQS Lerp(const VQS &a, const VQS &b, float ratio);

private:

  glm::vec3 Scale;
  glm::vec3 Position;
  Quaternion Rotation;

  glm::mat4 Matrix;
  bool NeedUpdate;
};

#endif