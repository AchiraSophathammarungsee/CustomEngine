/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology. 
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited. 
File Name: light.h
Purpose: enable light with multiple casting type and shadow mapping
Language: C++
Platform: visual studio, require OpenGL 4.6+, Windows 10
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 180002517
Creation date: 9/12/2019
End Header --------------------------------------------------------*/

#ifndef LIGHT_H
#define LIGHT_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <string>
#include "Rect.h"
#include "Object.h"
#include "BoundingVolume.h"
#include "Frustum.h"

class Framebuffer;

enum LightType
{
  LT_Directional, LT_Point, LT_Spotlight, LT_Skylight, LT_COUNT
};

enum LightMobility
{
  LM_Static, LM_Stationary, LM_Dynamic, LM_COUNT
};

class Shader;
class Model;
class Framebuffer;
class Texture;
class Camera;

class Light : public Object
{
public:
  Light(LightType type, const std::string &name = "unnamed light", glm::vec3 color = glm::vec3(1.f), float intensity = 1.f, bool castshadow = false, bool transShadow = false, bool colorShadow = false, unsigned int shadowResolution = 1024, LightMobility mobility = LM_Dynamic);
  virtual ~Light();

  virtual void UpdateUniforms(int shaderID, int id) = 0;

  LightType GetLightType() { return type; };

  LightMobility GetLightMobility() { return mobility; };
  void SetLightMobility(LightMobility mob) { mobility = mob; };

  const glm::vec3 &GetDiffuseColor() { return colorDiffuse; };
  virtual void SetDiffuseColor(glm::vec3 &col) { colorDiffuse = col; };
  const glm::vec3& GetAmbientColor() { return colorAmbient; };
  void SetAmbientColor(glm::vec3& col) { colorAmbient = col; };
  const glm::vec3& GetSpecularColor() { return colorSpecular; };
  void SetSpecularColor(glm::vec3& col) { colorSpecular = col; };

  void SetColor(glm::vec3& col) { colorDiffuse = col; colorSpecular = col; colorAmbient = col * 0.1f; }

  float GetIntensity() const { return intensity; };
  void SetIntensity(float val) { intensity = val; };

  float GetBias() const { return shadowBias; };
  void SetBias(float val) { shadowBias = val; };

  bool IsCastShadow() const { return castShadow; };
  bool HaveTransparentShadow() const { return transparentShadow; };
  bool HaveColorShadow() const { return colorShadow; };
  void SetCastShadow(bool cast) { castShadow = cast; };
  bool IsDisable() { return disable; };
  void SetDisable(bool flag) { disable = flag; };

  //void StartCaptureDepthMap(Shader *shader, Camera *camera);
  //void EndCaptureDepthMap();
  //void StartRenderScene(Shader* shader, int offset);

  virtual bool FrustumCull(const BoundingVolume& bd) const = 0;

  void IncrShadowUpdateTimer(float dt) { shadowUpdateTimer += dt; };
  void ResetShadowUpdateTimer() { shadowUpdateTimer = 0; };
  void SetShadowUpdateRate(float val) { shadowUpdateRate = val; };
  float GetShadowUpdateTimer() const { return shadowUpdateTimer; };
  float GetShadowUpdateRate() const { return shadowUpdateRate; };

  BoundingVolume GetBoundingVolume() const { return bv; };

  int skippedShadowFrames;

  virtual void DrawDebugMenu() override;
protected:
  virtual void SetupShadowDepthMap() {};
  void ClearShadowDepthMap();

  LightType type;
  LightMobility mobility;
  glm::vec3 colorDiffuse;
  glm::vec3 colorAmbient;
  glm::vec3 colorSpecular;
  float intensity;
  bool castShadow;
  bool transparentShadow;
  bool colorShadow;
  bool needUpdateMatrix;
  unsigned int shadowResolution;
  bool disable;
  float shadowBias;
  BoundingVolume bv;
  bool needShadowUpdate;
  float shadowUpdateRate;
  float shadowUpdateTimer;
};

class DirectionalLight : public Light
{
public:
  DirectionalLight(const std::string& name, glm::vec3 direction, glm::vec3 color = glm::vec3(1.f), float intensity = 1.f, bool castshadow = false, bool transShadow = false, bool colorShadow = false, unsigned int shadowResolution = 1024, unsigned int cascadeNum = 3, LightMobility mobility = LM_Dynamic);
  ~DirectionalLight() override;

  void UpdateUniforms(int shaderID, int id) override;

  const glm::vec3& GetDirection() { return direction; };
  void SetDirection(glm::vec3& dir) { direction = dir; };

  const glm::mat4& GetLightSpaceMatrix(unsigned int index) const { return lightSpaceMatrix[index]; };
  const std::vector<glm::mat4>& GetLightSpaceMatrixArray() const { return lightSpaceMatrix; };
  void CalculateLightSpaceMatrix(glm::vec3 center, float radius, int cascadeID);
  void CalculateLightSpaceMatrix();

  void SetFrustumSize(float size) { LightFrustumSize = size; needUpdateMatrix = true; };
  void SetFrustumFar(float f) { LightFrustumFar = f; needUpdateMatrix = true; };
  void SetFrustumNear(float n) { LightFrustumNear = n; needUpdateMatrix = true; };
  float GetFrustumFar() const { return LightFrustumFar; };
  float GetFrustumNear() const { return LightFrustumNear; };

  bool FrustumCull(const BoundingVolume& bd) const override;
  const Frustum& GetFrustum(int cascadeID) const { return frustum[cascadeID]; };

  void CaptureOpaqueShadow(glm::vec3 center, float radius, int cascadeID);
  void CaptureTransparentShadow();
  void ConvertToVSM(Model* quad, Shader*vsmConvert);
  void FilterVSMShadow(Model *quad, Shader* copyShader, Shader *blurShader);

  Texture* GetOpaqueShadowTex();
  void BindOpaqueShadowTex(int bindID);
  void BindTransparentShadowTex(int bindID);

  void DrawDebugMenu() override;
private:
  void SetupShadowDepthMap() override;
  
  glm::vec3 direction;
  std::vector<glm::mat4> lightSpaceMatrix;
  glm::mat4 lightSpaceProjection;
  glm::mat4 rotationMat;

  int cascadeNum;
  std::vector<float> cascadeDepthSlices;
  glm::vec3 prevCenter;

  Framebuffer* shadow;
  Framebuffer* StochasticShadow;
  Framebuffer* VSMShadow;
  Framebuffer* VSMBlurShadow;
  Framebuffer* VSMFinalShadow;
  float vsmBlurAmount;

  float LightFrustumSize;
  float LightFrustumFar;
  float LightFrustumNear;
  std::vector<Frustum> frustum;
};

class PointLight : public Light
{
public:
  PointLight(const std::string& name, glm::vec3 position, float radius = 8.f, glm::vec3 color = glm::vec3(1.f), float intensity = 1.f, bool castshadow = false, bool transShadow = false, bool colorShadow = false, unsigned int shadowResolution = 512, LightMobility mobility = LM_Dynamic);
  ~PointLight() override;

  void UpdateUniforms(int shaderID, int id) override;

  const glm::vec3& GetPosition() { return position; };
  void SetPosition(glm::vec3& pos) { position = pos; needUpdateMatrix = true; };

  void SetDiffuseColor(glm::vec3& col) override { colorDiffuse = col; UpdateRadius(); };

  float GetRadius() const { return radius; };
  void SetRadius(float val) { radius = val; UpdateProjMatrix(); };

  bool FrustumCull(const BoundingVolume& bd) const override;

  const std::vector<glm::mat4> &GetCubeSpaceMatrices() const { return ligthSpaceFinalMatrices; };

  void DrawDebugMenu() override;
private:
  void SetupShadowDepthMap() override;
  void UpdateRadius();
  void CalculateLightSpaceMatrix(Camera* camera);
  void UpdateProjMatrix();

  glm::vec3 position;
  float radius;
  Framebuffer* shadow;
  std::vector<glm::mat4> ligthSpaceFinalMatrices;
  glm::mat4 ligthSpaceViewMatrices[6];
  glm::mat4 lightSpaceProjection;
};

class SpotLight : public Light
{
public:
  SpotLight(const std::string& name, glm::vec3 position, glm::vec3 direction, float innerfalloff = 18.f, float outerfalloff = 25.f, float constant = 1.0f, float linear = 0.14f, float quadratic = 0.07f, glm::vec3 color = glm::vec3(1.f), float intensity = 1.f, bool castshadow = false, bool transShadow = false, bool colorShadow = false, unsigned int shadowResolution = 512, LightMobility mobility = LM_Dynamic);
  ~SpotLight() override;

  void UpdateUniforms(int shaderID, int id) override;

  const glm::vec3& GetPosition() { return position; };
  void SetPosition(glm::vec3& pos) { position = pos; };

  const glm::vec3& GetDirection() { return direction; };
  void SetDirection(glm::vec3& dir) { direction = dir; };

  float GetInnerFalloff() { return innerFalloff; };
  void SetInnerFalloff(float falloff) { innerFalloff = falloff; };

  float GetOuterFalloff() { return outerFalloff; };
  void SetOuterFalloff(float falloff) { outerFalloff = falloff; };

  float GetConstant() { return constant; };
  void SetConstant(float con) { constant = con; };

  float GetLinear() { return linear; };
  void SetLinear(float lin) { linear = lin; };

  float GetQuadratic() { return quadratic; };
  void SetQuadratic(float quad) { quadratic = quad; };

  float GetNearPlane() { return nearPlane; };
  float GetFarPlane() { return farPlane; };

  bool FrustumCull(const BoundingVolume& bd) const override;

  void DrawDebugMenu() override;

private:
  void CalculateLightSpaceMatrix(Camera* camera);

  glm::vec3 position, direction;
  float innerFalloff, outerFalloff;
  float constant, linear, quadratic;
  float nearPlane, farPlane;
};

#endif //LIGHT_H
