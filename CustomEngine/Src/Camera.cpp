/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Camera.cpp
Purpose: Encapsulate camera-related behaviours (projections, moving, zooming, etc)
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/12/2019
End Header --------------------------------------------------------*/

#include "Camera.h"
#include "EngineCore.h"
#include "Renderer.h"
#include "Window.h"
#include <GLFW/glfw3.h>

const float YAW = -90.f;
const float PITCH = 0.0f;

Camera::Camera(const std::string &name, float windowWidth, float windowHeight, float _near, float _far, bool orthographic, glm::vec3 pos, glm::vec3 target, glm::vec3 up) :
  Object(name), position(pos), up(up), target(target), needUpdate(true), projectionMatrix(glm::mat4(1.f)), lookAtMatrix(glm::mat4(1.f)), isOrthographic(orthographic), FOV(45.f),
  winHeight(windowHeight), winWidth(windowWidth), isFirstMouseInput(true), yaw(YAW), pitch(PITCH), near(_near), far(_far), finalMatrix(glm::mat4(1.f)), lastCursorPos(glm::vec2(0.f)), 
  lockControl(true), mouseLook(false), sensitivity(0.12f), frustumChanged(true), updateFrustrum(true)
{
  UpdateProjection();

  UpdateMatrix();
}

void Camera::SetFOV(float fov)
{
  FOV = fov;
  UpdateProjection();
  frustumChanged = true;
  needUpdate = true;
}

void Camera::UpdateProjection(void)
{
  float halfWidth = winWidth / 2.f;
  float halfHeight = winHeight / 2.f;
  float aspectRatio = winWidth / winHeight;

  if (isOrthographic)
  {
    float zoom = (FOV / 45.f);
    projectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, near, far);
  }
  else
  {
    projectionMatrix = glm::perspective(glm::radians(FOV), aspectRatio, near, far);
  }

  invProjection = glm::inverse(projectionMatrix);

  if (updateFrustrum) frustrum.UpdateProjection(FOV, aspectRatio, near, far);
}

void Camera::UpdateMatrix(void)
{
  if (needUpdate)
  {
    lookAtMatrix = glm::lookAt(position, position + target, up);
    finalMatrix = projectionMatrix * lookAtMatrix;

    invFinal = glm::inverse(finalMatrix);

    if(updateFrustrum) frustrum.UpdateLookAt(position, glm::normalize(target), up);

    needUpdate = false;
  }
}

void Camera::KeyInput(GLFWwindow* window)
{
  float cameraSpeed = 0.05f;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  {
    position += cameraSpeed * target;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
  {
    position -= cameraSpeed * target;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
  {
    glm::vec3 sideVec = cameraSpeed * glm::normalize(glm::cross(target, up));
    position -= sideVec;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
  {
    glm::vec3 sideVec = cameraSpeed * glm::normalize(glm::cross(target, up));
    position += sideVec;
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
  {
    position += cameraSpeed * up;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
  {
    position -= cameraSpeed * up;
  }

  needUpdate = true;
}

void Camera::MouseInput(float xpos, float ypos)
{
  if (isFirstMouseInput)
  {
    lastCursorPos.x = xpos;
    lastCursorPos.y = ypos;
    isFirstMouseInput = false;
  }

  float xoffset = xpos - lastCursorPos.x;
  float yoffset = lastCursorPos.y - ypos;
  lastCursorPos.x = xpos;
  lastCursorPos.y = ypos;

  if (mouseLook == false) return;

  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  target = glm::normalize(front);

  needUpdate = true;
}

void Camera::ScrollInput(double x, double y)
{
  if (FOV >= 1.0f && FOV <= 45.0f)
    FOV -= (float)y;
  if (FOV <= 1.0f)
    FOV = 1.0f;
  if (FOV >= 45.0f)
    FOV = 45.0f;

  SetFOV(FOV);
  needUpdate = true;
}

Ray Camera::MouseRayToWorld(bool& valid)
{
  double x, y;
  Window::GetCursorPos(x, y);

  // if outside of render view
  const Rect<unsigned int> &rect = Renderer::GetViewRect();
  if (rect.IsInRect(x, y) == false)
  {
    // return invalid ray
    valid = false;
    return Ray(glm::vec3(), glm::vec3());
  }

  // translate xy into renderer view
  x -= rect.TopLeftX();
  y -= rect.TopLeftY();

  // convert from screenspace [0,width] into ndc space [-1,1]
  x = ((x / Renderer::Width()) * 2.0) - 1.0;
  y = ((y / Renderer::Height()) * 2.0) - 1.0;

  LOG_TRACE_S("Camera", "Mouse pos: ({}, {})", x, y);

  glm::vec4 nearPos = invFinal * glm::vec4(x, 1.0 - y, 1.f, 1.f);
  glm::vec4 farPos = invFinal * glm::vec4(x, 1.0 - y, 0.f, 1.f);
  
  glm::vec3 pos = glm::vec3(nearPos.x, nearPos.y, nearPos.z);
  glm::vec3 fpos = glm::vec3(farPos.x, farPos.y, farPos.z);

  

  fpos -= pos;
  valid = true;
  return Ray(pos, fpos);
}

void Camera::DrawDebugMenu()
{
  Object::DrawDebugMenu();

  ImGui::InputFloat("Window Width", &winWidth);
  ImGui::InputFloat("Window Height", &winHeight);
  if (ImGui::InputFloat("Near", &near)) SetNear(near);
  if (ImGui::InputFloat("Far", &far)) SetFar(far);
  if (ImGui::InputFloat("FOV", &FOV)) SetFOV(FOV);
  ImGui::InputFloat("Sensitivity", &sensitivity);

  static const char* projectiontypes[] = {"Orthographic", "Perspective"};
  static int projection = 0;
  if (ImGui::ListBox("Projection", &projection, projectiontypes, 2))
  {
    if (projection)
    {
      isOrthographic = false;
    }
    else
    {
      isOrthographic = true;
    }
    UpdateProjection();
  }

  if (projection)
  {
    ImGui::InputFloat3("Position", &position[0]);
    ImGui::InputFloat3("Target", &target[0]);
    ImGui::InputFloat3("Up", &up[0]);
  }
  else
  {
    ImGui::InputFloat3("Position", &position[0]);
  }

  ImGui::Checkbox("Update frustrum", &updateFrustrum);
}