#pragma once
#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>

struct Plane3D
{
  Plane3D() : normal(), d() {};
  
  Plane3D(const glm::vec3 &norm, float d) : normal(norm), d(d) {};

  Plane3D(const glm::vec3& point, const glm::vec3& normal)
  {
    FromPointNormal(point, normal);
  };

  Plane3D(const glm::vec3& point, const glm::vec3& tangent, const glm::vec3& bitangent)
  {
    From3Points(point, tangent, bitangent);
  };

  void FromPointNormal(const glm::vec3& point, const glm::vec3& normal)
  {
    this->normal = normal;
    d = -glm::dot(point, normal);
  };

  void From3Points(const glm::vec3& point, const glm::vec3& tangent, const glm::vec3& bitangent)
  {
    FromPointNormal(point, glm::normalize(glm::cross(tangent - point, bitangent - point)));
  };

  float UnsignedDist(const glm::vec3& p) const
  {
    return glm::abs(normal.x * p.x + normal.y * p.y + normal.z * p.z + d);
  }

  float Dist(const glm::vec3& p) const
  {
    return normal.x * p.x + normal.y * p.y + normal.z * p.z + d;
  }

  glm::vec3 normal;
  float d;
};

#endif