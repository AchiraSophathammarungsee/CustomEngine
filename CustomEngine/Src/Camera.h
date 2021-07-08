/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology. 
Reproduction or disclosure of this file or its contents without the prior written 
consent of DigiPen Institute of Technology is prohibited. 
File Name: Camera.h
Purpose: Encapsulate camera-related behaviours (projections, moving, zooming, etc)
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517> 
Creation date: 9/12/2019
End Header --------------------------------------------------------*/

#ifndef CAMERA_H
#define CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include "Object.h"
#include "Frustum.h"
#include "Ray.h"

struct GLFWwindow;

class Camera : public Object
{
public:
  Camera(const std::string& name, float windowWidth, float windowHeight, float near, float far, bool orthographic, glm::vec3 pos = glm::vec3(0.f), glm::vec3 target = glm::vec3(0.f, 0.f, -1.f), glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
  ~Camera() {};

  void SetPosition(const glm::vec3& pos) { position = pos; needUpdate = true; };
  void SetTarget(const glm::vec3& _target) { target = _target; needUpdate = true; };
  void SetUp(const glm::vec3& _up) { up = _up; needUpdate = true; };
  const glm::vec3& GetPosition(void) { return position; };
  const glm::vec3& GetTarget(void) { return target; };
  const glm::vec3& GetUp(void) { return up; };

  void SetFOV(float fov);
  float GetFOV(void) { return FOV; };

  void UpdateProjection(void);
  void UpdateMatrix(void);

  const Frustum &GetFrustrum() const { return frustrum; };

  const glm::mat4& GetFinalMatrix(void) const { return finalMatrix; };
  const glm::mat4& GetLookAtMatrix(void) const { return lookAtMatrix; };
  const glm::mat4& GetProjMatrix(void) const { return projectionMatrix; };
  const glm::mat4& GetInvFinalMatrix(void) const { return invFinal; };
  const glm::mat4& GetInvProjectionMatrix(void) const { return invProjection; };

  void SetNear(float f) { near = f; UpdateProjection(); frustumChanged = true; needUpdate = true; };
  void SetFar(float f) { far = f; UpdateProjection(); frustumChanged = true; needUpdate = true; };
  float GetNear(void) const { return near; };
  float GetFar(void) const { return far; };

  void KeyInput(GLFWwindow* window);
  void MouseInput(float x, float y);
  void ScrollInput(double x, double y);
  void LockControl(bool lock) { lockControl = lock; };
  Ray MouseRayToWorld(bool &valid);

  bool MouseLook() { return mouseLook; };
  void SetMouseLook(bool flag) { mouseLook = flag; };

  bool IsFrustumChanged() const { return frustumChanged; };
  void ResetFrustumChanged() { frustumChanged = false; };

  void DrawDebugMenu() override;
private:
  float winHeight, winWidth;
  float near, far;

  glm::vec3 position;
  glm::vec3 target;
  glm::vec3 up;
  float FOV;

  bool updateFrustrum;
  Frustum frustrum;

  glm::mat4 lookAtMatrix;
  glm::mat4 projectionMatrix;
  glm::mat4 finalMatrix;
  glm::mat4 invProjection;
  glm::mat4 invFinal;
  bool needUpdate;
  bool isOrthographic;

  bool isFirstMouseInput;
  glm::vec2 lastCursorPos;
  float yaw, pitch;

  float sensitivity;
  bool lockControl;
  bool mouseLook;

  bool frustumChanged;
};

#endif //TRANSFORM_H
