#include "SphericalHarmonics.h"
#include <vector>
#define M_PI 3.14159265358979323846

std::vector<float> GetBase(const glm::vec3& direction) 
{
  float x = direction.x;
  float y = direction.y;
  float z = direction.z;
  std::vector<float> base(9, 0);

  // band 1
  base[0] = 1;

  // band 2
  base[1] = y;
  base[2] = z;
  base[3] = x;

  // band 3
  base[4] = x*y;
  base[5] = y*z;
  base[6] = 3*z*z-1;
  base[7] = x*z;
  base[8] = x*x-y*y;

  return base;
}

void SH::SphericalHarmonics(const float* pixels, int width, int height, SH_coeff& output)
{
  double area = 0.f;
  float hf = M_PI / height;
  float wf = 2.0 * M_PI / width;
  for (int y = 0; y < height; y++)
  {
    float phi = hf * float(y);
    float sinPhi = sin(phi) * hf * wf;

    for (int x = 0; x < width; x++)
    {
      float theta = wf * float(x);
      glm::vec3 norm(cos(theta)*sin(phi), sin(theta)*sin(phi), cos(phi));
      norm = glm::normalize(norm);

      std::vector<float> base = GetBase(norm);

      for (int i = 0; i < 9; i++)
      {
        base[i] *= sinPhi;
        output[i].r += base[i] * pixels[y * width * 3 + x * 3];
        output[i].g += base[i] * pixels[y * width * 3 + x * 3 + 1];
        output[i].b += base[i] * pixels[y * width * 3 + x * 3 + 2];
      }
      area += sinPhi;
    }
  }

  std::vector<float> coeff(9, 0);
  coeff[0] = 0.282095f * 3.141593f;
  coeff[1] = -0.488603f * 2.094395f;
  coeff[2] = 0.488603f * 2.094395f;
  coeff[3] = -0.488603f * 2.094395f;
  coeff[4] = 1.092548f * 0.785398f;
  coeff[5] = -1.092548f * 0.785398f;
  coeff[6] = 0.315392f * 0.785398f;
  coeff[7] = -1.092548f * 0.785398f;
  coeff[8] = 0.546274f * 0.785398f;

  for (int i = 0; i < 9; i++)
  {
    output[i] *= coeff[i];
  }
}