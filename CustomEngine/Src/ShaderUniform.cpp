#include "shaderUniform.h"
#include <imgui.h>
#include "ResourceManager.h"
#include "Cubemap.h"

void boolUniform::DrawDebugMenu()
{
  ImGui::Checkbox(name, &value);
}
void intUniform::DrawDebugMenu()
{
  ImGui::InputInt(name, &value);
}
void floatUniform::DrawDebugMenu()
{
  ImGui::InputFloat(name, &value);
}
void vec2Uniform::DrawDebugMenu()
{
  ImGui::InputFloat2(name, &value[0]);
}
void vec3Uniform::DrawDebugMenu()
{
  if (isColor)
    ImGui::ColorEdit3(name, &value[0]);
  else
    ImGui::InputFloat3(name, &value[0]);
}
void vec4Uniform::DrawDebugMenu()
{
  if (isColor)
    ImGui::ColorEdit4(name, &value[0]);
  else
    ImGui::InputFloat4(name, &value[0]);
}
void mat3Uniform::DrawDebugMenu()
{
  ImGui::Text(name);
  ImGui::InputFloat3("###mat30", &value[0][0]);
  ImGui::InputFloat3("###mat31", &value[0][0]);
  ImGui::InputFloat3("###mat32", &value[0][0]);
}
void mat4Uniform::DrawDebugMenu()
{
  ImGui::Text(name);
  ImGui::InputFloat4("###mat40", &value[0][0]);
  ImGui::InputFloat4("###mat41", &value[1][0]);
  ImGui::InputFloat4("###mat42", &value[2][0]);
  ImGui::InputFloat4("###mat43", &value[3][0]);
}
void tex1DUniform::DrawDebugMenu()
{
  static bool selectWindow = false;
  static Texture** selectTex = nullptr;
  ImGui::Text(name);
  if (value == nullptr)
  {
    if (ImGui::Button("Select texture"))
    {
      selectTex = &value;
      selectWindow = true;
    }
  }
  else
  {
    if (ImGui::ImageButton(ImTextureID(value->GetID()), ImVec2(80.f, 80.f)))
    {
      selectTex = &value;
      selectWindow = true;
    }
  }
  if (selectWindow)
  {
    ResourceManager::DrawResourceSelector<Texture>(selectTex, &selectWindow);
  }
}
void tex2DUniform::DrawDebugMenu()
{
  static bool selectWindow = false;
  static Texture** selectTex = nullptr;
  ImGui::Text(name);
  if (value == nullptr)
  {
    if (ImGui::Button("Select texture"))
    {
      selectTex = &value;
      selectWindow = true;
    }
  }
  else
  {
    if (ImGui::ImageButton(ImTextureID(value->GetID()), ImVec2(80.f, 80.f)))
    {
      selectTex = &value;
      selectWindow = true;
    }
  }
  if (selectWindow)
  {
    ResourceManager::DrawResourceSelector<Texture>(selectTex, &selectWindow);
  }
}
void tex3DUniform::DrawDebugMenu()
{
  static bool selectWindow = false;
  static Texture** selectTex = nullptr;
  ImGui::Text(name);
  if (value == nullptr)
  {
    if (ImGui::Button("Select texture"))
    {
      selectTex = &value;
      selectWindow = true;
    }
  }
  else
  {
    if (ImGui::ImageButton(ImTextureID(value->GetID()), ImVec2(80.f, 80.f)))
    {
      selectTex = &value;
      selectWindow = true;
    }
  }
  if (selectWindow)
  {
    ResourceManager::DrawResourceSelector<Texture>(selectTex, &selectWindow);
  }
}
void texCubeUniform::DrawDebugMenu()
{
  static bool selectWindow = false;
  static CubeMap** selectTex = nullptr;
  ImGui::Text(name);
  if (value)
    ImGui::Text("Current cubemap: %s", value->GetPath().c_str());
  else
    ImGui::Text("Current cubemap: null");

  if (ImGui::Button("Select cubemap"))
  {
    selectTex = &value;
    selectWindow = true;
  }
  if (selectWindow)
  {
    ResourceManager::DrawResourceSelector<CubeMap>(selectTex, &selectWindow);
  }
}