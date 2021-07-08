#define M_PI 3.1415926535897932384626433832795

float uniformRand(vec3 hashInput)
{
  return fract((sin(dot(hashInput.xyz, vec3(12.9898,78.233,34.2343))) * 0.5 + 0.5) * 43758.5453);
}

float DistSquared(vec3 a, vec3 b)
{
  a -= b;
  return dot(a, a);
}