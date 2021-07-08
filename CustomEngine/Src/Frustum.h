#pragma once
#ifndef FRUSTRUM_H
#define FRUSTRUM_H

#include <glm/glm.hpp>
#include "Plane.h"
#include "Bitmask.h"

struct FrustumCullResult
{
  bool outside;
  Bitmask insections;
};

struct Frustum
{
private:
  enum sides { PLANE_RIGHT = 0, PLANE_LEFT, PLANE_TOP, PLANE_BOTTOM, PLANE_NEAR, PLANE_FAR };

public:
  enum CullResults { CR_Right, CR_Left, CR_Top, CR_Bottom, CR_Near, CR_Far, CR_COUNT };

  Plane3D planes[6];

  glm::vec3 fc, nc;

  glm::vec3 fbl, fbr, ftl, ftr;
  glm::vec3 nbl, nbr, ntl, ntr;

  float nearD, farD, ratio, angle, tang;
  float nw, nh, fw, fh;

  void UpdateProjection(float angle, float ratio, float nearD, float farD);
  void UpdateOrtho(float width, float height, float nearD, float farD);
  void UpdateLookAt(const glm::vec3& camPos, const glm::vec3& viewDir, const glm::vec3& upVec);

  bool FastCullSphere(const glm::vec3& center, float radius) const;
  bool FastCullAABB(const glm::vec3& max, const glm::vec3& min) const;
  bool CullSphere(const glm::vec3 &center, float radius, Bitmask &insections) const;
  bool CullAABB(const glm::vec3& max, const glm::vec3& min, Bitmask& insections) const;
};

#endif