/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Transform.h
Purpose: Encapsulate all transformation behaviours (position, rotation, scale, matrix, etc)
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/11/2019
End Header --------------------------------------------------------*/

#ifndef TRANSFORM_H
#define TRANSFORM_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

class Transform
{
public:
  Transform(glm::vec3 pos = glm::vec3(0.f), glm::vec3 rotAxis = glm::vec3(0.f, 1.f, 0.f), glm::vec3 scale = glm::vec3(1.f), float rotAngle = 0.f);
  ~Transform() {};

  void SetPosition(const glm::vec3& pos) { position = pos; needUpdate = true; };
  void SetRotationAxis(const glm::vec3& axis) { rotationAxis = axis; needUpdate = true; };
  void SetScale(const glm::vec3& _scale) { scale = _scale; needUpdate = true; };
  void SetRotationAngle(const float degree) { rotationAngle = (degree > 360.f) ? degree - 360.f : degree; needUpdate = true; };

  void UpdateMatrix(void);

  const glm::vec3& GetPosition(void) const { return position; };
  const glm::vec3& GetRotationAxis(void) const { return rotationAxis; };
  const glm::vec3& GetScale(void) const { return scale; };
  const float GetRotationAngle(void) const { return rotationAngle; };

  const glm::mat4& GetMatrix(void) const { return transformMatrix; };
  void SetMatrix(const glm::mat4& mat) { transformMatrix = mat; };

  bool NeedUpdate() const { return needUpdate; };

  void DrawDebugMenu(void);
private:
  glm::vec3 position;
  glm::vec3 rotationAxis;
  glm::vec3 scale;
  float rotationAngle;

  glm::mat4 transformMatrix;
  bool needUpdate;
};

#endif //TRANSFORM_H
