/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Material.cpp
Purpose: encompass textures, uv transform, Diffuse, specular and ambient color
Language: C++
Platform: visual studio, require OpenGL 4.6+, Windows 10
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 180002517
Creation date: 9/14/2019
End Header --------------------------------------------------------*/

#include "Material.h"
#include <glad/glad.h>
#include "Texture.h"
#include "shader.hpp"
#include "ResourceManager.h"
#include "Framebuffer.h"
#include "Cubemap.h"

#define PREVIEWRESOLUTION 200
Model* Material::previewMesh;
//MaterialCreator* Material::materialCreators[MC_COUNT] = { new MatUnlitCreator(), new MatClustPhongCreator(), new MatClustPhongAlphaCreator(), new MatSkyboxCubeCreator() };
//static const char* matClassNames[MC_COUNT] = { "Unlit Color", "Clustered Phong", "Clustered Phong Alpha", "Skybox Cubemap" };

Material::Material(Shader* shader): shader(shader), cullMode(ShaderConfig::CULL_Back), blendMode(ShaderConfig::BLEND_Opaque)
{
  SyncShader();
}

void Material::SyncShader()
{
  auto &bools = shader->GetBoolProperties();
  auto &ints = shader->GetIntProperties();
  auto &floats = shader->GetFloatProperties();
  auto &vec2s = shader->GetVec2Properties();
  auto &vec3s = shader->GetVec3Properties();
  auto &vec4s = shader->GetVec4Properties();
  auto &tex1Ds = shader->GetSampler1DProperties();
  auto &tex2Ds = shader->GetSampler2DProperties();
  auto &tex3Ds = shader->GetSampler3DProperties();
  auto &texCubes = shader->GetSamplerCubeProperties();

  boolProperties.clear();
  intProperties.clear();
  floatProperties.clear();
  vec2Properties.clear();
  vec3Properties.clear();
  vec4Properties.clear();
  sampler1DProperties.clear();
  sampler2DProperties.clear();
  sampler3DProperties.clear();
  samplerCubeProperties.clear();

  for (const boolUniform& u : bools)
  {
    boolProperties[u.name] = u;
  }
  for (const intUniform& u : ints)
  {
    intProperties[u.name] = u;
  }
  for (const floatUniform& u : floats)
  {
    floatProperties[u.name] = u;
  }
  for (const vec2Uniform& u : vec2s)
  {
    vec2Properties[u.name] = u;
  }
  for (const vec3Uniform& u : vec3s)
  {
    vec3Properties[u.name] = u;
  }
  for (const vec4Uniform& u : vec4s)
  {
    vec4Properties[u.name] = u;
  }
  for (const tex1DUniform& u : tex1Ds)
  {
    sampler1DProperties[u.name] = u;
  }
  for (const tex2DUniform& u : tex2Ds)
  {
    sampler2DProperties[u.name] = u;
  }
  for (const tex3DUniform& u : tex3Ds)
  {
    sampler3DProperties[u.name] = u;
  }
  for (const texCubeUniform& u : texCubes)
  {
    samplerCubeProperties[u.name] = u;
  }
}

void Material::UpdateUniform()
{
  for (auto& it : boolProperties)
  {
    shader->SetBool(it.second);
  }
  for (auto& it : intProperties)
  {
    shader->SetInt(it.second);
  }
  for (auto& it : floatProperties)
  {
    shader->SetFloat(it.second);
  }
  for (auto& it : vec2Properties)
  {
    shader->SetVec2(it.second);
  }
  for (auto& it : vec3Properties)
  {
    shader->SetVec3(it.second);
  }
  for (auto& it : vec4Properties)
  {
    shader->SetVec4(it.second);
  }
  for (auto& it : sampler1DProperties)
  {
    if (it.second.value)
    {
      it.second.value->Bind(it.second.bindID);
      shader->SetSampler1D(it.second);
    }
  }
  for (auto& it : sampler2DProperties)
  {
    if (it.second.value)
    {
      it.second.value->Bind(it.second.bindID);
      shader->SetSampler2D(it.second);
    }
  }
  for (auto& it : sampler3DProperties)
  {
    if (it.second.value)
    {
      it.second.value->Bind(it.second.bindID);
      shader->SetSampler3D(it.second);
    }
  }
  for (auto& it : samplerCubeProperties)
  {
    if (it.second.value)
    {
      it.second.value->Bind(it.second.bindID);
      shader->SetSamplerCube(it.second);
    }
  }
}

void Material::UpdateUniform(Shader *_shader)
{
  for (auto& it : boolProperties)
  {
    _shader->SetBool(it.second);
  }
  for (auto& it : intProperties)
  {
    _shader->SetInt(it.second);
  }
  for (auto& it : floatProperties)
  {
    _shader->SetFloat(it.second);
  }
  for (auto& it : vec2Properties)
  {
    _shader->SetVec2(it.second);
  }
  for (auto& it : vec3Properties)
  {
    _shader->SetVec3(it.second);
  }
  for (auto& it : vec4Properties)
  {
    _shader->SetVec4(it.second);
  }
  for (auto& it : sampler1DProperties)
  {
    if (it.second.value)
    {
      it.second.value->Bind(it.second.bindID);
      _shader->SetSampler1D(it.second);
    }
  }
  for (auto& it : sampler2DProperties)
  {
    if (it.second.value)
    {
      it.second.value->Bind(it.second.bindID);
      _shader->SetSampler2D(it.second);
    }
  }
  for (auto& it : sampler3DProperties)
  {
    if (it.second.value)
    {
      it.second.value->Bind(it.second.bindID);
      _shader->SetSampler3D(it.second);
    }
  }
  for (auto& it : samplerCubeProperties)
  {
    if (it.second.value)
    {
      it.second.value->Bind(it.second.bindID);
      _shader->SetSamplerCube(it.second);
    }
  }
}

void Material::SetBool(const char* name, bool value)
{
  auto it = boolProperties.find(name);
  if (it == boolProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to set bool invalid property: {}", shader->GetName(), name);
    return;
  }
  it->second.value = value;
}
void Material::SetInt(const char* name, int value)
{
  auto it = intProperties.find(name);
  if (it == intProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to set int invalid property: {}", shader->GetName(), name);
    return;
  }
  it->second.value = value;
}
void Material::SetFloat(const char* name, float value)
{
  auto it = floatProperties.find(name);
  if (it == floatProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to set float invalid property: {}", shader->GetName(), name);
    return;
  }
  it->second.value = value;
}
void Material::SetVec2(const char* name, const glm::vec2& value)
{
  auto it = vec2Properties.find(name);
  if (it == vec2Properties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to set vec2 invalid property: {}", shader->GetName(), name);
    return;
  }
  it->second.value = value;
}
void Material::SetVec3(const char* name, const glm::vec3& value)
{
  auto it = vec3Properties.find(name);
  if (it == vec3Properties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to set vec3 invalid property: {}", shader->GetName(), name);
    return;
  }
  it->second.value = value;
}
void Material::SetVec4(const char* name, const glm::vec4& value)
{
  auto it = vec4Properties.find(name);
  if (it == vec4Properties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to set vec4 invalid property: {}", shader->GetName(), name);
    return;
  }
  it->second.value = value;
}
void Material::SetSampler1D(const char* name, Texture* tex)
{
  auto it = sampler1DProperties.find(name);
  if (it == sampler1DProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to set tex1D invalid property: {}", shader->GetName(), name);
    return;
  }
  it->second.value = tex;
}
void Material::SetSampler2D(const char* name, Texture* tex)
{
  auto it = sampler2DProperties.find(name);
  if (it == sampler2DProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to set tex2D invalid property: {}", shader->GetName(), name);
    return;
  }
  it->second.value = tex;
}
void Material::SetSampler3D(const char* name, Texture* tex)
{
  auto it = sampler3DProperties.find(name);
  if (it == sampler3DProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to set tex3D invalid property: {}", shader->GetName(), name);
    return;
  }
  it->second.value = tex;
}
void Material::SetSamplerCube(const char* name, CubeMap* tex)
{
  auto it = samplerCubeProperties.find(name);
  if (it == samplerCubeProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to set texCube invalid property: {}", shader->GetName(), name);
    return;
  }
  it->second.value = tex;
}

bool Material::GetBool(const char* name) const
{
  auto it = boolProperties.find(name);
  if (it == boolProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to get bool invalid property: {}", shader->GetName(), name);
    return false;
  }
  return it->second.value;
}
int Material::GetInt(const char* name)const
{
  auto it = intProperties.find(name);
  if (it == intProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to get bool invalid property: {}", shader->GetName(), name);
    return 0;
  }
  return it->second.value;
}
float Material::GetFloat(const char* name)const
{
  auto it = floatProperties.find(name);
  if (it == floatProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to get bool invalid property: {}", shader->GetName(), name);
    return 0.f;
  }
  return it->second.value;
}
glm::vec2 Material::GetVec2(const char* name)const
{
  auto it = vec2Properties.find(name);
  if (it == vec2Properties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to get bool invalid property: {}", shader->GetName(), name);
    return glm::vec2();
  }
  return it->second.value;
}
glm::vec3 Material::GetVec3(const char* name)const
{
  auto it = vec3Properties.find(name);
  if (it == vec3Properties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to get bool invalid property: {}", shader->GetName(), name);
    return glm::vec3();
  }
  return it->second.value;
}
glm::vec4 Material::GetVec4(const char* name)const
{
  auto it = vec4Properties.find(name);
  if (it == vec4Properties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to get bool invalid property: {}", shader->GetName(), name);
    return glm::vec4();
  }
  return it->second.value;
}
int Material::GetSampler1D(const char* name)const
{
  auto it = sampler1DProperties.find(name);
  if (it == sampler1DProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to get bool invalid property: {}", shader->GetName(), name);
    return -1;
  }
  return it->second.bindID;
}
int Material::GetSampler2D(const char* name)const
{
  auto it = sampler2DProperties.find(name);
  if (it == sampler2DProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to get bool invalid property: {}", shader->GetName(), name);
    return-1;
  }
  return it->second.bindID;
}
int Material::GetSampler3D(const char* name)const
{
  auto it = sampler3DProperties.find(name);
  if (it == sampler3DProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to get bool invalid property: {}", shader->GetName(), name);
    return-1;
  }
  return it->second.bindID;
}
int Material::GetSamplerCube(const char* name)const
{
  auto it = samplerCubeProperties.find(name);
  if (it == samplerCubeProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to get bool invalid property: {}", shader->GetName(), name);
    return -1;
  }
  return it->second.bindID;
}
Texture *Material::GetSampler2DTex(const char* name)
{
  auto it = sampler2DProperties.find(name);
  if (it == sampler2DProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to get bool invalid property: {}", shader->GetName(), name);
    return nullptr;
  }
  return it->second.value;
}
CubeMap* Material::GetSamplerCubeTex(const char* name)
{
  auto it = samplerCubeProperties.find(name);
  if (it == samplerCubeProperties.end())
  {
    LOG_ERROR_S("Material", "({}) tried to get bool invalid property: {}", shader->GetName(), name);
    return nullptr;
  }
  return it->second.value;
}

void Material::DrawDebugMenu()
{
  ImGui::Text("Shader: %s", shader->GetName().c_str());

  ImGui::ListBox("Cull Mode", (int*)&cullMode, ShaderConfig::CullModeString, ShaderConfig::CULL_COUNT);
  ImGui::ListBox("Blend Mode", (int*)&blendMode, ShaderConfig::BlendModeString, ShaderConfig::BLEND_COUNT);

  ImGui::Text("Properties:");
  ImGui::Separator();
  for (auto &it : boolProperties)
  {
    it.second.DrawDebugMenu();
  }
  for (auto& it : intProperties)
  {
    it.second.DrawDebugMenu();
  }
  for (auto& it : floatProperties)
  {
    it.second.DrawDebugMenu();
  }
  for (auto& it : vec2Properties)
  {
    it.second.DrawDebugMenu();
  }
  for (auto& it : vec3Properties)
  {
    it.second.DrawDebugMenu();
  }
  for (auto& it : vec4Properties)
  {
    it.second.DrawDebugMenu();
  }
  for (auto& it : sampler1DProperties)
  {
    it.second.DrawDebugMenu();
  }
  for (auto& it : sampler2DProperties)
  {
    it.second.DrawDebugMenu();
  }
  for (auto& it : sampler3DProperties)
  {
    it.second.DrawDebugMenu();
  }
  for (auto& it : samplerCubeProperties)
  {
    it.second.DrawDebugMenu();
  }
}

void Material::DrawResourceCreator(bool& openFlag)
{
  static char nameBuffer[256] = "unnamed";

  ImGui::SetNextWindowSize(ImVec2(400, 300));
  ImGui::Begin("New material", &openFlag);

  ImGui::InputText("Material Name", nameBuffer, 255);
  
  static bool selectShader = false;
  static Shader* newShader = ResourceManager::Get<Shader>("Cluster Phong");
  if(newShader)
    ImGui::Text("Current Shader: %s", newShader->GetName().c_str());
  else
    ImGui::Text("Current Shader: null");

  if (ImGui::Button("Select Shader"))
  {
    selectShader = true;
  }
  if (selectShader)
  {
    ResourceManager::DrawResourceSelector<Shader>(&newShader, &selectShader);
  }

  if (ImGui::Button("Create Material"))
  {
    ResourceManager::Add<Material>(new Material(newShader), nameBuffer);
    openFlag = false;
  }

  ImGui::End();
}