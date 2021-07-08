#include "Collider.h"
#include "EngineCore.h"
#include <numeric>

glm::vec3 Collider::BoxVertices[8] = { 
                                        glm::vec3(0.5f, 0.5f, 0.5f), 
                                        glm::vec3(-0.5f, 0.5f, 0.5f), 
                                        glm::vec3(0.5f, -0.5f, 0.5f), 
                                        glm::vec3(0.5f, 0.5f, -0.5f),
                                        glm::vec3(-0.5f, -0.5f, 0.5f),
                                        glm::vec3(0.5f, -0.5f, -0.5f),
                                        glm::vec3(-0.5f, 0.5f, -0.5f),
                                        glm::vec3(-0.5f, -0.5f, -0.5f)
                                     };

void Collider::UpdateTransform(Transform& parentTransform)
{
  if (NeedUpdate)
  {
    ModelTransform.SetPosition(Offset);
    //ModelTransform.SetScale(Scale);
    ModelTransform.SetRotationAxis(RotAxis);
    ModelTransform.SetRotationAngle(Angle);
    ModelTransform.UpdateMatrix();

    WorldTransform.SetMatrix(parentTransform.GetMatrix() * ModelTransform.GetMatrix());

    NeedUpdate = false;
  }
}

void ColliderBox::UpdateBB()
{
  constexpr float maxFloat = std::numeric_limits<float>::max();
  /*BB.bottom = maxFloat;
  BB.top = -maxFloat;
  BB.left = maxFloat;
  BB.right = -maxFloat;
  BB.back = maxFloat;
  BB.front = -maxFloat;

  for (int i = 0; i < 8; i++)
  {
    BoundingVertices[i] = WorldTransform.GetMatrix() * glm::vec4(BoxVertices[i], 1.f);

    BB.bottom = (BoundingVertices[i].y < BB.bottom) ? BoundingVertices[i].y : BB.bottom;
    BB.top = (BoundingVertices[i].y > BB.top) ? BoundingVertices[i].y : BB.top;
    BB.left = (BoundingVertices[i].x < BB.left) ? BoundingVertices[i].x : BB.left;
    BB.right = (BoundingVertices[i].x > BB.right) ? BoundingVertices[i].x : BB.right;
    BB.back = (BoundingVertices[i].z < BB.back) ? BoundingVertices[i].z : BB.back;
    BB.front = (BoundingVertices[i].z > BB.front) ? BoundingVertices[i].z : BB.front;
  }

  BB.UpdateCenter();
  BB.UpdateMinMax();*/
}

BoundingBox ColliderBox::BoundBox() const
{
  BoundingBox result;
  glm::vec3 halfScale = Scale / 2.f;

  float l, r, b, t, f, n;
  l = Position().x - halfScale.x;
  r = Position().x + halfScale.x;
  b = Position().y - halfScale.y;
  t = Position().y + halfScale.y;
  n = Position().z - halfScale.z;
  f = Position().z + halfScale.z;

  result.UpdateVertices(l, r, b, t, n, f);

  return result;
}

void Collider::DrawDebugElements()
{
  static const char* colliderTypes[Collider::Type::COUNT] = { "Box", "Sphere", "Capsule" };
  ImGui::Text("Type: %s", colliderTypes[ShapeType]);

  ImGui::InputFloat3("Offset", &Offset[0]);
  ImGui::InputFloat3("Rotation Axis", &RotAxis[0]);
  ImGui::InputFloat("Rotation Angle", &Angle);
  ImGui::Checkbox("Static", &Static);
  ImGui::Checkbox("Ghost", &Ghost);
  ImGui::Checkbox("Disable", &Disable);

  ImGui::Checkbox("Show Debug", &ShowDebug);
}

void ColliderBox::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("Collider Box"))
  {
    ImGui::Separator();
    ImGui::Indent(10.f);

    DrawDebugElements();

    ImGui::InputFloat3("Scale", &Scale[0]);

    ImGui::Unindent(10.f);
    ImGui::Separator();
  }
}

void ColliderSphere::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("Collider Sphere"))
  {
    ImGui::Separator();
    ImGui::Indent(10.f);

    DrawDebugElements();

    ImGui::InputFloat("Radius", &Radius);

    ImGui::Unindent(10.f);
    ImGui::Separator();
  }
}

void ColliderCapsule::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("Collider Capsule"))
  {
    ImGui::Separator();
    ImGui::Indent(10.f);

    DrawDebugElements();

    ImGui::InputFloat("Radius", &Radius);
    ImGui::InputFloat3("Begin", &Begin[0]);
    ImGui::InputFloat3("End", &End[0]);

    ImGui::Unindent(10.f);
    ImGui::Separator();
  }
}