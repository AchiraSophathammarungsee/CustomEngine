/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: UVWrap.cpp
Purpose: Encapsulate all uv wrapping behaviours
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_2
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 11/10/2019
End Header --------------------------------------------------------*/

#include "UVWrap.h"
#include <glad/glad.h>
#include "EngineCore.h"
#include "Model.h"
#include "shader.hpp"
#define M_PI 3.14159265358979323846

UVWrap::UVWrap(UVTypes _type, BoundingBox box, glm::vec2 offset, glm::vec2 scale) : type(_type), Offset(offset), Scale(scale), TextureEntityPosition(true), BB(box)
{}

void UVWrap::CalculateUVontoModel(std::vector<Vertex>& verticesData)
{
  /*glm::vec3 boundMax = glm::vec3(BB.right, BB.top, BB.front);
  glm::vec3 boundMin = glm::vec3(BB.left, BB.bottom, BB.back);*/

  switch (type)
  {
  case(UVT_Vertex):
  {
    break;
  }
  case(UVT_Planar):
  {
    if (TextureEntityPosition)
    {
      for (Vertex& v : verticesData)
      {
        v.uv.x = (1.f + v.position.x) / 2.f;
        v.uv.y = (1.f + v.position.y) / 2.f;

        glm::vec3 absVec = glm::abs(v.position);

        if (absVec.x >= absVec.y && absVec.x >= absVec.z)
        {
          v.uv.x = (v.position.x < 0.0f) ? v.position.z : -v.position.z;
          v.uv.y = v.position.y;
        }
        else if (absVec.y >= absVec.x && absVec.y >= absVec.z)
        {
          v.uv.x = (v.position.y < 0.0f) ? v.position.x : -v.position.x;
          v.uv.y = v.position.z;
        }
        else
        {
          v.uv.x = (v.position.z < 0.0f) ? v.position.x : -v.position.x;
          v.uv.y = v.position.y;
        }

        v.uv = (v.uv + glm::vec2(1.f)) / 2.f;
      }
    }
    else
    {
      for (Vertex& v : verticesData)
      {
        v.uv.x = (1.f + v.normal.x) / 2.f;
        v.uv.y = (1.f + v.normal.y) / 2.f;

        glm::vec3 absVec = glm::abs(v.normal);

        if (absVec.x >= absVec.y && absVec.x >= absVec.z)
        {
          v.uv.x = (v.normal.x < 0.0f) ? v.normal.z : -v.normal.z;
          v.uv.y = v.normal.y;
        }
        else if (absVec.y >= absVec.x && absVec.y >= absVec.z)
        {
          v.uv.x = (v.normal.y < 0.0f) ? v.normal.x : -v.normal.x;
          v.uv.y = v.normal.z;
        }
        else
        {
          v.uv.x = (v.normal.z < 0.0f) ? v.normal.x : -v.normal.x;
          v.uv.y = v.normal.y;
        }

        v.uv = (v.uv + glm::vec2(1.f)) / 2.f;
      }
    }
    break;
  }
  case(UVT_Sphere):
  {
    /*glm::vec3 center = (boundMax + boundMin) / 2.f;

    if (TextureEntityPosition)
    {
      for (Vertex& v : verticesData)
      {
        glm::vec3 p = v.position - center;

        float r = sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
        float theta = atan2f(p.y, p.x);
        float phi = acosf(p.z / r);

        v.uv.x = static_cast<float>(theta / (2.f * M_PI));
        v.uv.y = static_cast<float>((M_PI - phi) / M_PI);
      }
    }
    else
    {
      for (Vertex& v : verticesData)
      {
        glm::vec3 p = v.normal - center;

        float r = sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
        float theta = atan2f(p.y, p.x);
        float phi = acosf(p.z / r);

        v.uv.x = static_cast<float>(theta / (2.f * M_PI));
        v.uv.y = static_cast<float>((M_PI - phi) / M_PI);
      }
    }*/
    
    break;
  }
  case(UVT_Cylinder):
  {
    /*glm::vec3 center = (boundMax + boundMin) / 2.f;
    boundMax -= center;
    boundMin -= center;

    if (TextureEntityPosition)
    {
      for (Vertex& v : verticesData)
      {
        glm::vec3 p = v.position - center;

        float theta = atan2f(v.position.y, v.position.x);
        float z = (v.position.z - boundMin.z) / (boundMax.z - boundMin.z);

        v.uv.x = static_cast<float>(theta / (2.f * M_PI));
        v.uv.y = z;
      }
    }
    else
    {
      for (Vertex& v : verticesData)
      {
        glm::vec3 p = v.normal - center;

        float theta = atan2f(v.normal.y, v.normal.x);
        float z = (v.normal.z - boundMin.z) / (boundMax.z - boundMin.z);

        v.uv.x = static_cast<float>(theta / (2.f * M_PI));
        v.uv.y = z;
      }
    }
    break;*/
  }
  }

  // use calculated uv on the vertices from now on
  type = UVT_Vertex;
}

void UVWrap::UpdateUniform(Shader *shader)
{
  shader->use();
  /*shader->setVec3("uvwrap.BBcenter", BB.center);
  shader->setVec3("uvwrap.BBmax", BB.max);
  shader->setVec3("uvwrap.BBmin", BB.min);*/
  shader->setInt("uvwrap.type", (int)type);
  shader->setBool("uvwrap.texEntityPos", TextureEntityPosition);
  shader->setVec2("uvwrap.UVscale", Scale);
  shader->setVec2("uvwrap.UVshift", Offset);
}

void UVWrap::DrawDebugMenu(Model* model)
{
  static const char* uvWrapTypes[UVT_COUNT] = { "Vertex", "Planar", "Sphere", "Cylinder" };
  static int _type = type;
  static bool useCPU = false;

  if (model->haveFineData())
  {
    if (ImGui::ListBox("UV Type", &_type, uvWrapTypes, (int)UVT_COUNT))
    {
      type = (UVTypes)_type;
      if (useCPU)
        model->CalculateUVfromUVWrap();
    }

    if (ImGui::Checkbox("Calculate UV on CPU", &useCPU))
    {
      if (useCPU)
        model->CalculateUVfromUVWrap();
    }
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "DEFAULT UV CALCULATE = GPU");
  }
  else
  {
    if (ImGui::ListBox("UV Type", &_type, uvWrapTypes, (int)UVT_COUNT))
    {
      type = (UVTypes)_type;
    }
  }

  if (ImGui::Button("Use Position as texture entity"))
  {
    TextureEntityPosition = true;
  }
  if (ImGui::Button("Use Normal as texture entity"))
  {
    TextureEntityPosition = false;
  }

  ImGui::Text("UV transform");
  if (ImGui::IsItemHovered())
  {
    ImGui::BeginTooltip();
    ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Apply transformation to UV coordinates");
    ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Scale: scaling texture");
    ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Offset: panning texture");
    ImGui::EndTooltip();
  }
  ImGui::InputFloat2("UV scale", &Scale[0]);
  ImGui::InputFloat2("UV offset", &Offset[0]);
}