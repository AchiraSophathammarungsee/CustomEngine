#include "Quaternion.h"
#include "EngineCore.h"
/* CONSTRUCTOR */

Quaternion::Quaternion() : value(0.f, 0.f, 0.f, 1.f), matrix(1.f), needUpdate(true)
{
  UpdateMatrix();
}

Quaternion::Quaternion(float i1, float i2, float i3, float real) : value(i1, i2, i3, real), matrix(1.f), needUpdate(true)
{
  UpdateMatrix();
}

Quaternion::Quaternion(float angleX, float angleY, float angleZ) : value(), matrix(1.f), needUpdate(true)
{
  FromEulerAngle(angleX, angleY, angleZ);
  UpdateMatrix();
}

Quaternion::Quaternion(const glm::vec3& axis, float angle) : value(), matrix(1.f), needUpdate(true)
{
  FromAxisAngle(axis, angle);
  UpdateMatrix();
}

Quaternion::Quaternion(const glm::mat4& rotationMat) : value(), matrix(1.f), needUpdate(true)
{
  FromRotMat(rotationMat);
  UpdateMatrix();
}

/* PUBLIC */
void Quaternion::Update()
{
  if (needUpdate)
  {
    UpdateMatrix();
  }
}

void Quaternion::Normalize()
{
  float length = this->Length();
  value.x /= length;
  value.y /= length;
  value.z /= length;
  value.w /= length;

  needUpdate = true;
}

void Quaternion::Conjugate()
{
  value.x = -value.x;
  value.y = -value.y;
  value.z = -value.z;

  needUpdate = true;
}

float Quaternion::Length() const
{
  return sqrtf(value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w);
}

Quaternion Quaternion::Inverse() const
{
  float lengthpow2 = value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w;

  return Quaternion(-value.x / lengthpow2, -value.y / lengthpow2, -value.z / lengthpow2, value.w / lengthpow2);
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
  float x = value.w * other.value.x + value.x * other.value.w + value.y * other.value.z - value.z * other.value.y;
  float y = value.w * other.value.y - value.x * other.value.z + value.y * other.value.w + value.z * other.value.x;
  float z = value.w * other.value.z + value.x * other.value.y - value.y * other.value.x + value.z * other.value.w;
  float w = value.w * other.value.w - value.x * other.value.x - value.y * other.value.y - value.z * other.value.z;
  return Quaternion(x,y,z,w);
}

glm::vec4 Quaternion::operator*(const glm::vec4& vec) const
{
  return matrix * vec;
}

Quaternion Quaternion::operator*(float scalar) const
{
  return Quaternion(value.x * scalar, value.y * scalar, value.z * scalar, value.w * scalar);
}

Quaternion Quaternion::operator/(float scalar) const
{
  return Quaternion(value.x / scalar, value.y / scalar, value.z / scalar, value.w / scalar);
}

Quaternion Quaternion::operator+(const Quaternion& other) const
{
  return Quaternion(value.x + other.value.x, value.y + other.value.y, value.z + other.value.z, value.w + other.value.w);
}

Quaternion Quaternion::operator-(const Quaternion& other) const
{
  return Quaternion(value.x - other.value.x, value.y - other.value.y, value.z - other.value.z, value.w - other.value.w);
}

Quaternion Quaternion::operator+(const glm::vec4& vec) const
{
  return Quaternion(value.x + vec.x, value.y + vec.y, value.z + vec.z, value.w + vec.w);
}

Quaternion Quaternion::operator-(const glm::vec4& vec) const
{
  return Quaternion(value.x - vec.x, value.y - vec.y, value.z - vec.z, value.w - vec.w);
}

float& Quaternion::operator[](unsigned i)
{
  ASSERT(i < 4, "Quaternion: out of bound index access!");

  return value[i];
}

/* PRIVATE */

void Quaternion::UpdateMatrix()
{
  matrix[0][0] = 1.f - 2.f * (value.y * value.y + value.z * value.z);
  matrix[1][1] = 1.f - 2.f * (value.x * value.x + value.z * value.z);
  matrix[2][2] = 1.f - 2.f * (value.x * value.x + value.y * value.y);

  matrix[0][1] = 2.f * (value.x * value.y - value.w * value.z);
  matrix[0][2] = 2.f * (value.x * value.z + value.w * value.y);
  matrix[1][0] = 2.f * (value.x * value.y + value.w * value.z);
  matrix[1][2] = 2.f * (value.y * value.z - value.w * value.x);
  matrix[2][0] = 2.f * (value.x * value.z - value.w * value.y);
  matrix[2][1] = 2.f * (value.y * value.z + value.w * value.x);

  matrix[0][3] = 0.f;
  matrix[1][3] = 0.f;
  matrix[2][3] = 0.f;
  matrix[3][0] = 0.f;
  matrix[3][1] = 0.f;
  matrix[3][2] = 0.f;
  matrix[3][3] = 1.f;

  matrix = glm::transpose(matrix);

  needUpdate = false;
}

void Quaternion::FromEulerAngle(float angleX, float angleY, float angleZ)
{
  float radX = glm::radians(angleX);
  float radY = glm::radians(angleY);
  float radZ = glm::radians(angleZ);

  float c1 = cos(radX / 2.f);
  float c2 = cos(radY / 2.f);
  float c3 = cos(radZ / 2.f);
  float s1 = sin(radX / 2.f);
  float s2 = sin(radY / 2.f);
  float s3 = sin(radZ / 2.f);

  value.x = s1 * s2 * c3 + c1 * c2 * s3;
  value.y = s1 * c2 * c3 + c1 * s2 * s3;
  value.z = c1 * s2 * c3 - s1 * c2 * s3;
  value.w = c1 * c2 * c3 - s1 * s2 * s3;

  needUpdate = true;
}

void Quaternion::FromAxisAngle(const glm::vec3& axis, float angle)
{
  glm::vec3 normAxis = (axis);
  float radAngle = glm::radians(angle);
  float sinAngle = sin(radAngle / 2.f);
  value.x = normAxis.x * sinAngle;
  value.y = normAxis.y * sinAngle;
  value.z = normAxis.z * sinAngle;
  value.w = cos(radAngle / 2.f);

  needUpdate = true;
}

void Quaternion::FromRotMat(const glm::mat4 & mat)
{
  value.w = 0.5f * sqrtf(mat[0][0] + mat[1][1] + mat[2][2] + 1.f);
  value.x = (mat[2][1] - mat[1][2]) / (4.f * value.w);
  value.y = (mat[0][2] - mat[2][0]) / (4.f * value.w);
  value.z = (mat[1][0] - mat[0][1]) / (4.f * value.w);

  needUpdate = true;
}

/* STATIC */

float Quaternion::Dot(const Quaternion& a, const Quaternion& b)
{
  return a.value.x * b.value.x + a.value.y * b.value.y + a.value.z * b.value.z + a.value.w * b.value.w;
}

Quaternion Quaternion::EulerAngle(float angleX, float angleY, float angleZ)
{
  return Quaternion(angleX, angleY, angleZ);
}

Quaternion Quaternion::AxisAngle(const glm::vec3& axis, float angle)
{
  return Quaternion(axis, angle);
}

Quaternion Quaternion::RotationMatrix(const glm::mat4& mat)
{
  return Quaternion(mat);
}

Quaternion Quaternion::Normalize(const Quaternion& a)
{
  float length = a.Length();
  return Quaternion(a.value.x / length, a.value.y / length, a.value.z / length, a.value.w / length);
}

Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float t)
{
  float dot = Quaternion::Dot(a, b);

  // If too close, i.e. nearly q0=±q1 do linear interpolation (and normalize length)
  if (dot > 0.9995f)
  {
    return Quaternion::Normalize((a + (b - a) * t));
  }

  Quaternion qa = a;
  Quaternion qb = b;

  // If interpolation would go the “long way around”
  // change the sign (of either one)
  if (dot < 0.0f)
  {
    qa = a * -1;
    dot = -dot;
  }

  float angle = glm::acos(dot);

  return (qa * glm::sin((1.f - t) * angle) + qb * glm::sin(t * angle)) / (glm::sin(angle));
}