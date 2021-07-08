
#ifndef COLLIDER_H
#define COLLIDER_H

#include <vector>
#include <glm/glm.hpp>
#include "Transform.h"
#include "BoundingBox.h"

class Collider
{
public:
  enum Type { Box, Sphere, Capsule, COUNT };

  Collider(Collider::Type type = Box, const glm::vec3& offset = glm::vec3(), float angle = 0.f, const glm::vec3& rotAxis = glm::vec3(0, 1, 0), bool _static = false)
    : ShapeType(type), Offset(offset), Angle(angle), RotAxis(rotAxis), Static(_static), Ghost(false), Disable(false), NeedUpdate(true), ShowDebug(false) {};

  Transform WorldTransform;
  Transform ModelTransform;
  glm::vec3 Center;
  glm::vec3 Offset;
  glm::vec3 RotAxis;
  float Angle;

  bool Static;
  bool Ghost;
  bool Disable;
  bool ShowDebug;

  Collider::Type GetType() const { return ShapeType; };
  glm::vec3 Position() const { return Center + Offset; };

  void UpdateTransform(Transform& parentTransform);

  virtual void DrawDebugMenu() {};
  void DrawDebugElements();
  
protected:
  glm::vec3 BoundingVertices[8];
  static glm::vec3 BoxVertices[8];

private:
  
  bool NeedUpdate;
  Collider::Type ShapeType;
};

class ColliderBox : public Collider
{
public:
  ColliderBox(const glm::vec3& scale = glm::vec3(1.f), const glm::vec3& offset = glm::vec3(), float angle = 0.f, const glm::vec3& rotAxis = glm::vec3(0, 1, 0), bool _static = false)
    : Collider(Collider::Type::Box, offset, angle, rotAxis, _static) {};

  glm::vec3 Scale;
  //BoundingBox BB;

  void UpdateBB();
  BoundingBox BoundBox() const;
  void DrawDebugMenu() override;

private:

};

class ColliderSphere : public Collider
{
public:
  ColliderSphere(float radius = 1.f, const glm::vec3& offset = glm::vec3(), float angle = 0.f, const glm::vec3& rotAxis = glm::vec3(0, 1, 0), bool _static = false)
    : Collider(Collider::Type::Sphere, offset, angle, rotAxis, _static), Radius(radius) {};

  void DrawDebugMenu() override;

  float Radius;

private:

};

class ColliderCapsule : public Collider
{
public:
  ColliderCapsule(float radius = 1.f, const glm::vec3& offset = glm::vec3(), float angle = 0.f, const glm::vec3& rotAxis = glm::vec3(0, 1, 0), bool _static = false)
    : Collider(Collider::Type::Capsule, offset, angle, rotAxis, _static), Radius(radius) {};

  void DrawDebugMenu() override;

  glm::vec3 Begin;
  glm::vec3 End;
  float Radius;

private:

};

#endif