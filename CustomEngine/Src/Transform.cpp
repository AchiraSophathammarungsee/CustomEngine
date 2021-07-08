/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Transform.cpp
Purpose: Encapsulate all transformation behaviours (position, rotation, scale, matrix, etc)
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/11/2019
End Header --------------------------------------------------------*/

#include "Transform.h"
#include "EngineCore.h"

Transform::Transform(glm::vec3 pos, glm::vec3 rotAxis, glm::vec3 _scale, float rotAngle) :
  position(pos), rotationAxis(rotAxis), scale(_scale), rotationAngle(rotAngle), needUpdate(true), transformMatrix(glm::mat4(1.f))
{
  //UpdateMatrix();
}

void Transform::UpdateMatrix(void)
{
  if (needUpdate)
  {
    transformMatrix = glm::translate(position) * glm::rotate(glm::radians(rotationAngle), rotationAxis) * glm::scale(scale);
    needUpdate = false;
  }
}

void Transform::DrawDebugMenu(void)
{
  if (ImGui::CollapsingHeader("Transform"))
  {
    ImGui::Separator();
    ImGui::Indent(10.f);

    if (ImGui::InputFloat3("Position", &position[0]))
      needUpdate = true;
    if (ImGui::InputFloat3("Rotation Axis", &rotationAxis[0]))
      needUpdate = true;
    if (ImGui::InputFloat("Rotation Angle", &rotationAngle))
      needUpdate = true;
    if (ImGui::InputFloat3("Scale", &scale[0]))
      needUpdate = true;

    ImGui::Unindent(10.f);
    ImGui::Separator();
  }
}