#pragma once
#ifndef QUATERNION_H
#define QUATERNION_H

#include <glm/glm.hpp>

class Quaternion
{
public:
  Quaternion();
  Quaternion(float i1, float i2, float i3, float real);
  // in degrees
  Quaternion(float angleX, float angleY, float angleZ);
  // in degrees
  Quaternion(const glm::vec3 &axis, float angle);
  Quaternion(const glm::mat4 &rotationMat);

  void Conjugate();
  void Normalize();
  float Length() const;
  Quaternion Inverse() const;
  Quaternion operator*(const Quaternion& other) const;
  glm::vec4 operator*(const glm::vec4& vec) const;
  Quaternion operator*(float scalar) const;
  Quaternion operator/(float scalar) const;
  Quaternion operator+(const Quaternion& other) const;
  Quaternion operator-(const Quaternion& other) const;
  Quaternion operator+(const glm::vec4 &vec) const;
  Quaternion operator-(const glm::vec4& vec) const;
  float& operator[](unsigned i);

  void Update();
  const glm::mat4& GetMatrix() { Update(); return matrix; };
  const glm::vec4& GetVec() const { return value; };

  static float Dot(const Quaternion& a, const Quaternion& b);
  // in degrees
  static Quaternion EulerAngle(float angleX, float angleY, float angleZ);
  // in degrees
  static Quaternion AxisAngle(const glm::vec3& axis, float angle);
  static Quaternion RotationMatrix(const glm::mat4& mat);
  static Quaternion Normalize(const Quaternion& a);

  static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t);

private:
  void UpdateMatrix();
  void FromEulerAngle(float angleX, float angleY, float angleZ);
  void FromAxisAngle(const glm::vec3& axis, float angle);
  void FromRotMat(const glm::mat4 &mat);

  //[imaginaryX, imaginaryY, imaginaryZ, real]
  glm::vec4 value; 
  glm::mat4 matrix;
  bool needUpdate;

};

#endif