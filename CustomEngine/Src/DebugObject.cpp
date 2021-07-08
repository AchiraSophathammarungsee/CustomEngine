#include "DebugObject.h"
#include "Renderer.h"
#include "Model.h"

void DebugObject::Init(DebugObjectType _type, const Transform& _transform, const glm::vec3& _color)
{
  type = _type;
  transform = _transform;
  color = _color;
}

void DebugObject::UpdateUniform(Shader* shader)
{
  transform.UpdateMatrix();
  shader->setMat4("modelMatrix", transform.GetMatrix());
  shader->setVec3("color", color);
}

void DebugObject::InitPoints(const std::vector<glm::vec3>& point)
{
  vertexPoints.clear();
  vertexPoints = point;
}

void DebugObject::UpdateMesh(Model* model)
{
  std::vector<Vertex> &vert = model->GetVertices();
  int i = 0;
  for (Vertex& v : vert)
  {
    v.position = vertexPoints[i];
    i++;
  }
  model->UpdateBuffer();
}

Transform DebugModel::identityMat;
void DebugModel::InitTransform()
{
  identityMat.SetScale(glm::vec3(1.f));
  identityMat.UpdateMatrix();
}

void DebugModel::Init(Model* _model, const glm::vec3& _color)
{
  model = _model;
  color = _color;
}

void DebugModel::Draw(Shader* shader)
{
  shader->setMat4("modelMatrix", DebugModel::identityMat.GetMatrix());
  shader->setVec3("color", color);
  model->Draw();
}