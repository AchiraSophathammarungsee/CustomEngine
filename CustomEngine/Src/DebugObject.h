#ifndef DEBUGOBJECT_H
#define DEBUGOBJECT_H

#include "Transform.h"
#include <vector>

class Shader;
class Model;

enum DebugObjectType { DOT_Line, DOT_LineSquare, DOT_LineCircle, DOT_LineCube, DOT_LineSphere, DOT_Sphere, DOT_LineTriangle, DOT_CircleSphere, DOT_COUNT };

class DebugObject
{
public:
  DebugObject(DebugObjectType type = DOT_LineCube, const Transform& transform = Transform(), const glm::vec3& color = glm::vec3(1.f)) : type(type), transform(transform), color(color) {};

  void Init(DebugObjectType type, const Transform& transform, const glm::vec3& color);
  void InitPoints(const std::vector<glm::vec3> &point);
  void UpdateUniform(Shader *shader);
  void UpdateMesh(Model* model);

  DebugObjectType Type() { return type; };

  Transform transform;
  glm::vec3 color;
private:
  DebugObjectType type;
  std::vector<glm::vec3> vertexPoints;
};

class DebugModel
{
public:
  DebugModel(Model *model = nullptr, const glm::vec3& color = glm::vec3(1.f)) : model(model), color(color) {};
  ~DebugModel() {};

  void Init(Model* model, const glm::vec3& color);
  void Draw(Shader* shader);
  static void InitTransform();

  glm::vec3 color;
private:
  Model* model;
  static Transform identityMat;
};

#endif