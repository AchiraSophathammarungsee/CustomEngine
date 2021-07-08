#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <glm/glm.hpp>
#include <vector>
#include "Transform.h"
#include "Vertex.h"

struct BoundingBox
{
  BoundingBox()
  {
    BoxVertices.push_back(glm::vec3());
    BoxVertices.push_back(glm::vec3());
    BoxVertices.push_back(glm::vec3());
    BoxVertices.push_back(glm::vec3());
    BoxVertices.push_back(glm::vec3());
    BoxVertices.push_back(glm::vec3());
    BoxVertices.push_back(glm::vec3());
    BoxVertices.push_back(glm::vec3());

    InitVertices.push_back(glm::vec4());
    InitVertices.push_back(glm::vec4());
    InitVertices.push_back(glm::vec4());
    InitVertices.push_back(glm::vec4());
    InitVertices.push_back(glm::vec4());
    InitVertices.push_back(glm::vec4());
    InitVertices.push_back(glm::vec4());
    InitVertices.push_back(glm::vec4());
  }

  float Left() const { return InitVertices[0].x; };
  float Right() const { return InitVertices[7].x; };
  float Top() const { return InitVertices[0].y; };
  float Bottom() const { return InitVertices[7].y; };
  float Near() const { return InitVertices[0].z; };
  float Far() const { return InitVertices[7].z; };

  void UpdateVertices(const std::vector<glm::vec3> &vertices);
  void UpdateVertices(const std::vector<Vertex> &vertices);
  void UpdateVertices(float l, float r, float b, float t, float n, float f);
  void UpdateTransform(const Transform& transform, std::vector<glm::vec3> &out);

  std::vector<glm::vec4> InitVertices;
  std::vector<glm::vec3> BoxVertices;
};

#endif