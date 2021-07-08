#include "BoundingBox.h"
#include <numeric>

void BoundingBox::UpdateVertices(const std::vector<glm::vec3>& vertices)
{
  constexpr float maxFloat = std::numeric_limits<float>::max();

  glm::vec3 min(maxFloat);
  glm::vec3 max(-maxFloat);

  for (const glm::vec3& v : vertices)
  {
    min.x = fminf(v.x, min.x);
    min.y = fminf(v.y, min.y);
    min.z = fminf(v.z, min.z);

    max.x = fmaxf(v.x, max.x);
    max.y = fmaxf(v.y, max.y);
    max.z = fmaxf(v.z, max.z);
  }

  InitVertices[0] = glm::vec4(min.x, min.y, min.z, 1.f); // near bottom left
  InitVertices[1] = glm::vec4(max.x, min.y, min.z, 1.f); // near bottom right
  InitVertices[2] = glm::vec4(min.x, max.y, min.z, 1.f); // near top left
  InitVertices[3] = glm::vec4(max.x, max.y, min.z, 1.f); // near top right

  InitVertices[4] = glm::vec4(min.x, min.y, max.z, 1.f); // far bottom left
  InitVertices[5] = glm::vec4(max.x, min.y, max.z, 1.f); // far bottom right
  InitVertices[6] = glm::vec4(min.x, max.y, max.z, 1.f); // far top left
  InitVertices[7] = glm::vec4(min.x, max.y, max.z, 1.f); // far top right
}

void BoundingBox::UpdateVertices(const std::vector<Vertex>& vertices)
{
  constexpr float maxFloat = std::numeric_limits<float>::max();

  glm::vec3 min(maxFloat);
  glm::vec3 max(-maxFloat);

  for (const Vertex& v : vertices)
  {
    min.x = fminf(v.position.x, min.x);
    min.y = fminf(v.position.y, min.y);
    min.z = fminf(v.position.z, min.z);

    max.x = fmaxf(v.position.x, max.x);
    max.y = fmaxf(v.position.y, max.y);
    max.z = fmaxf(v.position.z, max.z);
  }

  InitVertices[0] = glm::vec4(min.x, min.y, min.z, 1.f); // near bottom left
  InitVertices[1] = glm::vec4(max.x, min.y, min.z, 1.f); // near bottom right
  InitVertices[2] = glm::vec4(min.x, max.y, min.z, 1.f); // near top left
  InitVertices[3] = glm::vec4(max.x, max.y, min.z, 1.f); // near top right

  InitVertices[4] = glm::vec4(min.x, min.y, max.z, 1.f); // far bottom left
  InitVertices[5] = glm::vec4(max.x, min.y, max.z, 1.f); // far bottom right
  InitVertices[6] = glm::vec4(min.x, max.y, max.z, 1.f); // far top left
  InitVertices[7] = glm::vec4(min.x, max.y, max.z, 1.f); // far top right
}

void BoundingBox::UpdateVertices(float l, float r, float b, float t, float n, float f)
{
  InitVertices[0] = glm::vec4(l, b, n, 1.f); // near bottom left
  InitVertices[1] = glm::vec4(r, b, n, 1.f); // near bottom right
  InitVertices[2] = glm::vec4(l, t, n, 1.f); // near top left
  InitVertices[3] = glm::vec4(r, t, n, 1.f); // near top right

  InitVertices[4] = glm::vec4(l, b, f, 1.f); // far bottom left
  InitVertices[5] = glm::vec4(r, b, f, 1.f); // far bottom right
  InitVertices[6] = glm::vec4(l, t, f, 1.f); // far top left
  InitVertices[7] = glm::vec4(r, t, f, 1.f); // far top right
}

void BoundingBox::UpdateTransform(const Transform& transform, std::vector<glm::vec3>& out)
{
  for (int i = 0; i < 8; i++)
  {
    glm::vec4 vert = transform.GetMatrix() * InitVertices[i];
    out[i].x = vert.x;
    out[i].y = vert.y;
    out[i].z = vert.z;
  }
}