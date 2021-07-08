#pragma once
#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <glm/glm.hpp>

namespace MathUtil
{
  double PI();

  void InitRand();

  int RandomRange(int min, int max);
  float RandomRange(float min, float max);
  double RandomRange(double min, double max);

  float SqrDist(const glm::vec2& vec);
  float SqrDist(const glm::vec3& vec);

  bool SolveQuadratic(const float& a, const float& b, const float& c, float& t0, float& t1);
}

#endif