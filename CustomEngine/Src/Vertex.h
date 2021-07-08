#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>
#include <vector>
#include "EngineCore.h"

#define MAX_BONES_PER_VERTEX 4

struct Vertex
{
  Vertex(glm::vec3 pos = glm::vec3(0.f), glm::vec3 norm = glm::vec3(0.f), glm::vec2 uv = glm::vec2(0.f)) : position(pos), normal(norm), uv(uv) {};

  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 bitangent;
  glm::vec2 uv;

  std::vector<unsigned int> sharingFaces;
};

struct SkinnedVertex
{
public:
  SkinnedVertex(glm::vec3 pos = glm::vec3(0.f), glm::vec3 norm = glm::vec3(0.f), glm::vec2 uv = glm::vec2(0.f)) : position(pos), normal(norm), uv(uv)
  {
    boneIDs[0] = 0;
    boneIDs[1] = 0;
    boneIDs[2] = 0;
    boneIDs[3] = 0;

    weights[0] = 0.f;
    weights[1] = 0.f;
    weights[2] = 0.f;
    weights[3] = 0.f;
  };

  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 bitangent;
  glm::vec2 uv;
  glm::vec4 boneIDs;
  glm::vec4 weights;
};

#endif