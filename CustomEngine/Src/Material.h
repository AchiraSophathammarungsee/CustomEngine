/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Material.h
Purpose: encompass textures, uv transform, diffuse, specular and ambient color
Language: C++
Platform: visual studio, require OpenGL 4.6+, Windows 10
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 180002517
Creation date: 9/14/2019
End Header --------------------------------------------------------*/

#ifndef MATERIAL_H
#define MATERIAL_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <string>
#include "ShaderConfig.h"
#include "shaderUniform.h"
#include <unordered_map>

class Texture;

enum ShadingModel
{
  SM_Phong, SM_PBR
};

enum BlendMode
{
  BM_Normal, BM_Add, BM_Multiply, BM_Screen
};

class Shader;
class Transform;
class Framebuffer;
class Model;
class Texture;
class CubeMap;

class Material
{
public:
  Material(Shader *shader = nullptr);
  ~Material() {};

  void SyncShader();
  void UpdateUniform();

  void UpdateUniform(Shader *shader);

  void DrawDebugMenu();

  Shader* GetShader() { return shader; };
  ShaderConfig::CullModes GetCullMode() const { return cullMode; };
  ShaderConfig::BlendMode GetBlendMode() const { return blendMode; };

  void SetCullMode(ShaderConfig::CullModes mode) { cullMode = mode; };
  void SetBlendMode(ShaderConfig::BlendMode mode) { blendMode = mode; };

  void SetBool(const char *name, bool value);
  void SetInt(const char* name, int value);
  void SetFloat(const char* name, float value);
  void SetVec2(const char* name, const glm::vec2& value);
  void SetVec3(const char* name, const glm::vec3& value);
  void SetVec4(const char* name, const glm::vec4& value);
  void SetSampler1D(const char* name, Texture *tex);
  void SetSampler2D(const char* name, Texture* tex);
  void SetSampler3D(const char* name, Texture* tex);
  void SetSamplerCube(const char* name, CubeMap* tex);

  bool GetBool(const char* name) const;
  int GetInt(const char* name)const;
  float GetFloat(const char* name)const;
  glm::vec2 GetVec2(const char* name)const;
  glm::vec3 GetVec3(const char* name)const;
  glm::vec4 GetVec4(const char* name)const;
  int GetSampler1D(const char* name)const;
  int GetSampler2D(const char* name)const;
  int GetSampler3D(const char* name)const;
  int GetSamplerCube(const char* name)const;
  Texture* GetSampler2DTex(const char* name);
  CubeMap* GetSamplerCubeTex(const char* name);

  static void GeneratePreview(Material* mat, Framebuffer* preview, Shader* previewShader, Transform* transform) {};
  static void SetPreviewMesh(Model* mesh) { previewMesh = mesh; };
  static void DrawResourceCreator(bool& openFlag);
  
protected:
  Shader* shader;
  ShaderConfig::CullModes cullMode;
  ShaderConfig::BlendMode blendMode;

  std::unordered_map<std::string, boolUniform> boolProperties;
  std::unordered_map<std::string, intUniform> intProperties;
  std::unordered_map<std::string, floatUniform> floatProperties;
  std::unordered_map<std::string, vec2Uniform> vec2Properties;
  std::unordered_map<std::string, vec3Uniform> vec3Properties;
  std::unordered_map<std::string, vec4Uniform> vec4Properties;
  std::unordered_map<std::string, tex1DUniform> sampler1DProperties;
  std::unordered_map<std::string, tex2DUniform> sampler2DProperties;
  std::unordered_map<std::string, tex3DUniform> sampler3DProperties;
  std::unordered_map<std::string, texCubeUniform> samplerCubeProperties;

  Framebuffer* preview;
  static Model* previewMesh;

  //static MaterialCreator* materialCreators[MC_COUNT];
};

#endif //MATERIAL_H
