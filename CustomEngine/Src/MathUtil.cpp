#include "MathUtil.h"
#include <algorithm>
#include <time.h>

#define D_PI 3.14159265358979323846

namespace MathUtil
{
  double PI()
  {
    return D_PI;
  }

  void InitRand()
  {
    srand(time(NULL));
  }

  int RandomRange(int min, int max)
  {
    return rand() % max + min;
  }

  float RandomRange(float min, float max)
  {
    return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
  }

  double RandomRange(double min, double max)
  {
    return min + static_cast <double> (rand()) / (static_cast <double> (RAND_MAX / (max - min)));
  }

  float SqrDist(const glm::vec2& vec)
  {
    return vec.x* vec.x + vec.y * vec.y;
  }

  float SqrDist(const glm::vec3& vec)
  {
    return vec.x* vec.x + vec.y * vec.y + vec.z * vec.z;
  }

  bool SolveQuadratic(const float& a, const float& b, const float& c, float& t0, float& t1)
  {
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) t0 = t1 = -0.5 * b / a;
    else {
      float q = (b > 0) ? -0.5 * (b + sqrt(discr)) : -0.5 * (b - sqrt(discr));
      t0 = q / a;
      t1 = c / q;
    }
    if (t0 > t1) std::swap(t0, t1);

    return true;
  }
}