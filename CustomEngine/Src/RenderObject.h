#ifndef RENDEROBJECT_H
#define RENDEROBJECT_H

#include "Transform.h"
#include <string>
#include "BoundingVolume.h"
#include "MeshEnum.h"
#include "ECS.h"

class Material;
class Shader;

class RenderObject
{
public:
  RenderObject() : transform(), material(nullptr)
  {
    boundingVolume = BoundingVolume(BVT_Sphere);
  }
  ~RenderObject() {};

  void Draw(Shader* shader);
  void UpdateMaterialUniforms();

  //void DrawDebugMenu();

  unsigned int VAO;
  unsigned int IndiceCount;
  PrimitiveMode primitiveMode;
  Transform transform;
  BoundingVolume boundingVolume;
  Material* material;
  std::vector<glm::mat4>* boneArray;
  EntityID entity;

private:
};

#endif