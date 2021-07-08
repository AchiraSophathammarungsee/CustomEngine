#pragma once
#ifndef SPHERICALHARMONICS_H
#define SPHERICALHARMONICS_H

#include <glm/glm.hpp>

namespace SH
{
  typedef glm::vec3 SH_coeff[9];

  void SphericalHarmonics(const float* pixels, int width, int height, SH_coeff &coeff);
}

#endif