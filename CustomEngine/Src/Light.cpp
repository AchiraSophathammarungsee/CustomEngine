/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: light.cpp
Purpose: enable light with multiple casting type and shadow mapping
Language: C++
Platform: visual studio, require OpenGL 4.6+, Windows 10
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 180002517
Creation date: 9/12/2019
End Header --------------------------------------------------------*/

#include "Light.h"
#include <glad/glad.h>
#include "Framebuffer.h"
#include "EngineCore.h"
#include "Texture.h"
#include "Camera.h"
#include "Renderer.h"
#include "shader.hpp"
#include "Cubemap.h"
#include "Model.h"
#include <numeric>

Light::Light(LightType _type, const std::string& name, glm::vec3 _color, float intensity, bool castshadow, bool transShadow, bool colorShadow, unsigned int shadowResolution, LightMobility _mobility) :
  Object(name), type(_type), castShadow(castshadow), colorDiffuse(_color), colorAmbient(_color * 0.1f), colorSpecular(_color), mobility(_mobility), shadowResolution(shadowResolution), 
  disable(false), needUpdateMatrix(true), shadowBias(0.009f), shadowUpdateTimer(0.f), shadowUpdateRate(1.f), transparentShadow(transShadow), colorShadow(colorShadow),
  intensity(intensity)
{
}

Light::~Light()
{
}

void Light::ClearShadowDepthMap()
{
}

void Light::DrawDebugMenu()
{
  Object::DrawDebugMenu();

  static const char* lighttypes[LT_COUNT] = { "Directional", "Point", "Spotlight", "Skylight" };

  ImGui::Text("Type: %s", lighttypes[type]);
  ImGui::InputFloat("Intensity", &intensity);
  ImGui::ColorEdit3("Dffuse Color", &colorDiffuse[0]);
  //ImGui::ColorEdit3("Ambient Color", &colorAmbient[0]);
  //ImGui::ColorEdit3("Specular Color", &colorSpecular[0]);

  ImGui::Checkbox("Cast Shadow", &castShadow);
  ImGui::Checkbox("Transparent Shadow", &transparentShadow);
  ImGui::Checkbox("Color Shadow", &colorShadow);
  ImGui::InputFloat("Shadow bias", &shadowBias);
  ImGui::Checkbox("Disable", &disable);
}

DirectionalLight::DirectionalLight(const std::string& name, glm::vec3 dir, glm::vec3 color, float intensity, bool castshadow, bool transShadow, bool colorShadow, unsigned int shadowResolution, unsigned int cascadeNum, LightMobility mobility) :
  direction(dir), Light(LT_Directional, name, color, intensity, castshadow, transShadow, colorShadow, shadowResolution, mobility), LightFrustumSize(8.f), LightFrustumFar(10.f), LightFrustumNear(-10),
  lightSpaceProjection(1.f), vsmBlurAmount(1.f), shadow(nullptr), StochasticShadow(nullptr), VSMShadow(nullptr), VSMBlurShadow(nullptr), VSMFinalShadow(nullptr), prevCenter(), cascadeNum(cascadeNum)
{
  SetupShadowDepthMap();
}

DirectionalLight::~DirectionalLight()
{
  /*if (castShadow)
  {
    delete shadow;
    if (transparentShadow)
    {
      delete StochasticShadow;
      delete VSMShadow;
      delete VSMBlurShadow;
      delete VSMFinalShadow;
    }
  }*/
}

void DirectionalLight::SetupShadowDepthMap()
{
  if (castShadow)
  {
    for (int i = 0; i < cascadeNum; i++)
    {
      lightSpaceMatrix.push_back(glm::mat4(1.f));
      frustum.push_back(Frustum());
    }

  //  // opaque
  //  shadow = new Framebuffer(0, 0, shadowResolution, shadowResolution);
  //  glBindFramebuffer(GL_FRAMEBUFFER, shadow->GetFBO());
  //  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  //  Texture* shadowMap = new Texture(nullptr, shadowResolution, shadowResolution, TT_2D, 4, TEB_Wrap, TAB_Nearest, TC_Depth, TC_Depth, DT_Float);
  //  shadowMap->SetBorderColor(borderColor);
  //  shadow->AddBufferTexture("depth map", shadowMap, BA_Depth, 0);

  //  if (transparentShadow)
  //  {
  //    TextureComponent format = (colorShadow) ? TC_RGB : TC_RG;
  //    TextureComponent internalFormat = (colorShadow) ? TC_RGB : TC_RG;

  //    // stochastic
  //    int upsample = 2;
  //    StochasticShadow = new Framebuffer(0, 0, shadowResolution * upsample, shadowResolution * upsample);
  //    glBindFramebuffer(GL_FRAMEBUFFER, StochasticShadow->GetFBO());
  //    Texture* transparentMap = new Texture(nullptr, shadowResolution * upsample, shadowResolution * upsample, TT_2D, 4, TEB_Wrap, TAB_Nearest, format, internalFormat, DT_Float);
  //    float borderColor2[] = { 0.0f, 0.0f, 0.0f, 0.0f };
  //    transparentMap->SetBorderColor(borderColor2);
  //    StochasticShadow->AddBufferTexture("transparent map", transparentMap, BA_Color, 0);
  //    glDrawBuffer(GL_COLOR_ATTACHMENT0);

  //    // VSM
  //    unsigned int vsmattachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  //    VSMShadow = new Framebuffer(0, 0, shadowResolution * upsample, shadowResolution * upsample);
  //    glBindFramebuffer(GL_FRAMEBUFFER, VSMShadow->GetFBO());
  //    VSMShadow->AddBufferTexture("mean", new Texture(nullptr, shadowResolution * upsample, shadowResolution * upsample, TT_2D, 4, TEB_Clamp, TAB_Linear, format, internalFormat, DT_Float), BA_Color, 0);
  //    if (colorShadow)
  //    {
  //      VSMShadow->AddBufferTexture("variance", new Texture(nullptr, shadowResolution * upsample, shadowResolution * upsample, TT_2D, 4, TEB_Clamp, TAB_Linear, format, internalFormat, DT_Float), BA_Color, 1);
  //      glDrawBuffers(2, vsmattachments);
  //    }
  //    else
  //    {
  //      glDrawBuffer(GL_COLOR_ATTACHMENT0);
  //    }

  //    // VSM Blur
  //    int downsample = 2;
  //    VSMBlurShadow = new Framebuffer(0, 0, shadowResolution / downsample, shadowResolution / downsample);
  //    glBindFramebuffer(GL_FRAMEBUFFER, VSMBlurShadow->GetFBO());
  //    VSMBlurShadow->AddBufferTexture("mean", new Texture(nullptr, shadowResolution / downsample, shadowResolution / downsample, TT_2D, 4, TEB_Clamp, TAB_Linear, format, internalFormat, DT_Float), BA_Color, 0);
  //    if (colorShadow)
  //    {
  //      VSMBlurShadow->AddBufferTexture("variance", new Texture(nullptr, shadowResolution / downsample, shadowResolution / downsample, TT_2D, 4, TEB_Clamp, TAB_Linear, format, internalFormat, DT_Float), BA_Color, 1);
  //      glDrawBuffers(2, vsmattachments);
  //    }
  //    else
  //    {
  //      glDrawBuffer(GL_COLOR_ATTACHMENT0);
  //    }

  //    // VSM Final
  //    VSMFinalShadow = new Framebuffer(0, 0, shadowResolution / downsample, shadowResolution / downsample);
  //    glBindFramebuffer(GL_FRAMEBUFFER, VSMFinalShadow->GetFBO());
  //    VSMFinalShadow->AddBufferTexture("mean", new Texture(nullptr, shadowResolution / downsample, shadowResolution / downsample, TT_2D, 4, TEB_Clamp, TAB_LinearMipmap, format, internalFormat, DT_Float), BA_Color, 0);
  //    if (colorShadow)
  //    {
  //      VSMFinalShadow->AddBufferTexture("variance", new Texture(nullptr, shadowResolution / downsample, shadowResolution / downsample, TT_2D, 4, TEB_Clamp, TAB_LinearMipmap, format, internalFormat, DT_Float), BA_Color, 1);
  //      glDrawBuffers(2, vsmattachments);
  //    }
  //    else
  //    {
  //      glDrawBuffer(GL_COLOR_ATTACHMENT0);
  //    }
  //  }
  }

  //glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DirectionalLight::CaptureOpaqueShadow(glm::vec3 center, float radius, int cascadeID)
{
  shadow->BeginCapture();
  CalculateLightSpaceMatrix(center, radius, cascadeID);
}

void DirectionalLight::CaptureTransparentShadow()
{
  VSMShadow->BeginCapture(false);
  static unsigned int vsmattachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, vsmattachments);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.f, 1.f, 1.f, 0.f)[0]);
  glClearBufferfv(GL_COLOR, 1, &glm::vec4(1.f, 1.f, 1.f, 0.f)[0]);
  
  //shadow->CopyDepthBuffer(StochasticShadow, 0, 0);
}

void DirectionalLight::ConvertToVSM(Model* quad, Shader* vsmConvert)
{
  VSMShadow->BeginCapture();
  static unsigned int vsmattachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, vsmattachments);
  vsmConvert->use();
  shadow->GetTexture(0).texture->Bind(0);
  vsmConvert->setInt("opaqueTex", 0);
  StochasticShadow->GetTexture(0).texture->Bind(1);
  vsmConvert->setInt("stochastTex", 1);
  quad->Draw();
}

void DirectionalLight::FilterVSMShadow(Model* quad, Shader* copyShader, Shader* blurShader)
{
  // down sample
  VSMFinalShadow->BeginCapture();
  static unsigned int vsmattachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, vsmattachments);
  copyShader->use();
  VSMShadow->GetTexture(0).texture->Bind(0);
  copyShader->setInt("mean", 0);
  if (colorShadow)
  {
    VSMShadow->GetTexture(1).texture->Bind(1);
    copyShader->setInt("variance", 1);
  }
  quad->Draw();

  blurShader->use();
  //blurShader->setVec2("nearFar", glm::vec2(LightFrustumNear, LightFrustumFar));
  blurShader->setInt("mean", 0);
  if (colorShadow) blurShader->setInt("variance", 1);

  // horizontal blur
  VSMBlurShadow->BeginCapture();
  glDrawBuffers(2, vsmattachments);
  VSMFinalShadow->GetTexture(0).texture->Bind(0);
  if (colorShadow) VSMFinalShadow->GetTexture(1).texture->Bind(1);
  blurShader->setVec2("blurScale", glm::vec2(vsmBlurAmount / (float)VSMFinalShadow->GetWidht(), 0.f));
  quad->Draw();

  // variance blur filter
  VSMFinalShadow->BeginCapture();
  glDrawBuffers(2, vsmattachments);
  VSMBlurShadow->GetTexture(0).texture->Bind(0);
  if (colorShadow) VSMBlurShadow->GetTexture(1).texture->Bind(1);
  blurShader->setVec2("blurScale", glm::vec2(0.f, vsmBlurAmount / (float)VSMFinalShadow->GetHeight()));
  quad->Draw();

  // generate mipmap
  VSMFinalShadow->GetTexture(0).texture->GenerateMipmap();
  if (colorShadow) VSMFinalShadow->GetTexture(1).texture->GenerateMipmap();
}

Texture* DirectionalLight::GetOpaqueShadowTex()
{
  return shadow->GetTexture(0).texture;
}

void DirectionalLight::BindOpaqueShadowTex(int bindID)
{
  shadow->GetTexture(0).texture->Bind(bindID);
}

void DirectionalLight::BindTransparentShadowTex(int bindID)
{
  VSMFinalShadow->GetTexture(0).texture->Bind(bindID);
  if (colorShadow) VSMFinalShadow->GetTexture(1).texture->Bind(bindID + 1);
}

void DirectionalLight::UpdateUniforms(int shaderID, int id)
{
  glUseProgram(shaderID);

  std::string arrayStr = "dirLights[" + std::to_string(id) + "].";

  GLuint loc = glGetUniformLocation(shaderID, (arrayStr + "direction").c_str());
  glUniform3fv(loc, 1, &direction[0]);
  loc = glGetUniformLocation(shaderID, (arrayStr + "diffuse").c_str());
  glUniform3fv(loc, 1, &GetDiffuseColor()[0]);
  loc = glGetUniformLocation(shaderID, (arrayStr + "ambient").c_str());
  glUniform3fv(loc, 1, &GetAmbientColor()[0]);
  loc = glGetUniformLocation(shaderID, (arrayStr + "specular").c_str());
  glUniform3fv(loc, 1, &GetSpecularColor()[0]);
}

void DirectionalLight::CalculateLightSpaceMatrix(glm::vec3 center, float radius, int cascadeID)
{
  if (needUpdateMatrix)
  {
    lightSpaceProjection = glm::ortho(-LightFrustumSize, LightFrustumSize, -LightFrustumSize, LightFrustumSize, LightFrustumNear, LightFrustumFar);

    frustum[cascadeID].UpdateOrtho(LightFrustumSize, LightFrustumSize, LightFrustumNear, LightFrustumFar);
  }

  glm::vec3 normDir = glm::normalize(direction);
  glm::mat4 lightView = glm::lookAt(center, center + normDir, glm::vec3(0.0f, 1.0f, 0.0f));
  lightSpaceMatrix[cascadeID] = lightSpaceProjection * lightView;

  frustum[cascadeID].UpdateLookAt(center, normDir, glm::vec3(0.0f, 1.0f, 0.0f));
}

void DirectionalLight::CalculateLightSpaceMatrix()
{
  if (needUpdateMatrix)
  {
    lightSpaceProjection = glm::ortho(-LightFrustumSize, LightFrustumSize, -LightFrustumSize, LightFrustumSize, LightFrustumNear, LightFrustumFar);
    frustum[0].UpdateOrtho(LightFrustumSize, LightFrustumSize, LightFrustumNear, LightFrustumFar);
  }

  glm::vec3 normDir = glm::normalize(direction);
  glm::mat4 lightView = glm::lookAt(glm::vec3(0), glm::vec3(0) + normDir, glm::vec3(0.0f, 1.0f, 0.0f));
  lightSpaceMatrix[0] = lightSpaceProjection * lightView;
  frustum[0].UpdateOrtho(LightFrustumSize, LightFrustumSize, LightFrustumNear, LightFrustumFar);
}

bool DirectionalLight::FrustumCull(const BoundingVolume& bd) const
{
  if (bd.GetType() == BoundingVolumeTypes::BVT_AABB)
  {

  }
  else
  {

  }
  return true;
}

void DirectionalLight::DrawDebugMenu()
{
  Light::DrawDebugMenu();

  ImGui::InputFloat3("Direction", &direction[0]);

  ImGui::Separator();
  ImGui::Text("Shadow map");
  ImGui::Text("Resolution: %d", shadowResolution);
  if (ImGui::InputFloat("Frustum size", &LightFrustumSize))
    SetFrustumSize(LightFrustumSize);
  if (ImGui::InputFloat("Frustum far plane", &LightFrustumFar))
    SetFrustumFar(LightFrustumFar);
  if (ImGui::InputFloat("Frustum near plane", &LightFrustumNear))
    SetFrustumNear(LightFrustumNear);
  ImGui::InputFloat("VSM Blur Amount", &vsmBlurAmount);
  //shadow->DrawDebugMenu();
  //StochasticShadow->DrawDebugMenu();
  //VSMShadow->DrawDebugMenu();
  //VSMBlurShadow->DrawDebugMenu();
  //VSMFinalShadow->DrawDebugMenu();
  ImGui::Separator();
}

PointLight::PointLight(const std::string& name, glm::vec3 position, float radius, glm::vec3 color, float intensity, bool castshadow, bool transShadow, bool colorShadow, unsigned int shadowResolution, LightMobility mobility) :
  position(position), radius(radius), Light(LT_Point, name, color, intensity, castshadow, transShadow, colorShadow, shadowResolution, mobility), lightSpaceProjection(1.f)
{
  bv = BoundingVolume(position, radius);
  if (castShadow == false) return;

  //static const float dirs[6][3] = { { 1.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, -1.f, 0.f }, { 0.f, 0.f, 1.f }, { 0.f, 0.f, -1.f } };
  ligthSpaceViewMatrices[0] = glm::lookAt(position, position + glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
  ligthSpaceViewMatrices[1] = glm::lookAt(position, position + glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
  ligthSpaceViewMatrices[2] = glm::lookAt(position, position + glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
  ligthSpaceViewMatrices[3] = glm::lookAt(position, position + glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
  ligthSpaceViewMatrices[4] = glm::lookAt(position, position + glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f));
  ligthSpaceViewMatrices[5] = glm::lookAt(position, position + glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f));
  for (int i = 0; i < 6; i++)
  {
    ligthSpaceFinalMatrices.push_back(glm::mat4(1.f));
  }
  SetupShadowDepthMap();
  UpdateProjMatrix();
  CalculateLightSpaceMatrix(nullptr);
}

PointLight::~PointLight()
{
  if (castShadow)
  {
    delete shadow;
  }
}

void PointLight::SetupShadowDepthMap()
{
  glBindFramebuffer(GL_FRAMEBUFFER, shadow->GetFBO());
  //glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  CubeMap* shadowMap = new CubeMap(shadowResolution, shadowResolution, TEB_Wrap, TAB_Linear, TC_RG, TC_RG32F, DT_Float);
  shadow->AddBufferTexture("depth map", shadowMap, BA_Color, 0);

  glDrawBuffer(GL_COLOR_ATTACHMENT0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PointLight::UpdateUniforms(int shaderID, int id)
{
  glUseProgram(shaderID);

  std::string arrayStr = "pointLights[" + std::to_string(id) + "].";

  GLuint loc = glGetUniformLocation(shaderID, (arrayStr + "position").c_str());
  glUniform3fv(loc, 1, &position[0]);
  loc = glGetUniformLocation(shaderID, (arrayStr + "diffuse").c_str());
  glUniform3fv(loc, 1, &GetDiffuseColor()[0]);
  loc = glGetUniformLocation(shaderID, (arrayStr + "ambient").c_str());
  glUniform3fv(loc, 1, &GetAmbientColor()[0]);
  loc = glGetUniformLocation(shaderID, (arrayStr + "specular").c_str());
  glUniform3fv(loc, 1, &GetSpecularColor()[0]);
  loc = glGetUniformLocation(shaderID, (arrayStr + "radius").c_str());
  glUniform1f(loc, radius);
}

void PointLight::UpdateProjMatrix()
{
  bv.radius = radius;
  lightSpaceProjection = glm::perspective(glm::radians(90.f), 1.f, 0.001f, radius);
  needUpdateMatrix = true;
}

void PointLight::CalculateLightSpaceMatrix(Camera* camera)
{
  if (needUpdateMatrix)
  {
    bv.center = position;

    ligthSpaceViewMatrices[0] = glm::lookAt(position, position + glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
    ligthSpaceViewMatrices[1] = glm::lookAt(position, position + glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
    ligthSpaceViewMatrices[2] = glm::lookAt(position, position + glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
    ligthSpaceViewMatrices[3] = glm::lookAt(position, position + glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
    ligthSpaceViewMatrices[4] = glm::lookAt(position, position + glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f));
    ligthSpaceViewMatrices[5] = glm::lookAt(position, position + glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f));

    ligthSpaceFinalMatrices[0] = lightSpaceProjection * ligthSpaceViewMatrices[0];
    ligthSpaceFinalMatrices[1] = lightSpaceProjection * ligthSpaceViewMatrices[1];
    ligthSpaceFinalMatrices[2] = lightSpaceProjection * ligthSpaceViewMatrices[2];
    ligthSpaceFinalMatrices[3] = lightSpaceProjection * ligthSpaceViewMatrices[3];
    ligthSpaceFinalMatrices[4] = lightSpaceProjection * ligthSpaceViewMatrices[4];
    ligthSpaceFinalMatrices[5] = lightSpaceProjection * ligthSpaceViewMatrices[5];

    needUpdateMatrix = false;
  }
}

void PointLight::UpdateRadius()
{
  //float lightMax = std::fmaxf(std::fmaxf(colorDiffuse.r, colorDiffuse.g), colorDiffuse.b);
  //radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax)))  / (2 * quadratic);
}

bool PointLight::FrustumCull(const BoundingVolume& bd) const
{
  if (bd.GetType() == BoundingVolumeTypes::BVT_AABB)
  {

  }
  else
  {

  }
  return true;
}

void PointLight::DrawDebugMenu()
{
  Light::DrawDebugMenu();

  if (ImGui::InputFloat3("Position", &position[0])) SetPosition(position);
  if (ImGui::InputFloat("Radius", &radius)) SetRadius(radius);

  ImGui::Text("Shadow Resolution: %d", (castShadow) ? shadow->GetWidht() : 0);
  ImGui::InputFloat("Shadow update rate", &shadowUpdateRate);
  ImGui::Text("Skipped shadow render frames: %d", skippedShadowFrames);
  /*ImGui::Separator();
  ImGui::Text("View matrices:");
  for (int i = 0; i < 6; i++)
  {
    ImGui::Text("|%f|%f|%f|%f|", ligthSpaceViewMatrices[i][0][0], ligthSpaceViewMatrices[i][0][1], ligthSpaceViewMatrices[i][0][2], ligthSpaceViewMatrices[i][0][3]);
    ImGui::Text("|%f|%f|%f|%f|", ligthSpaceViewMatrices[i][1][0], ligthSpaceViewMatrices[i][1][1], ligthSpaceViewMatrices[i][1][2], ligthSpaceViewMatrices[i][1][3]);
    ImGui::Text("|%f|%f|%f|%f|", ligthSpaceViewMatrices[i][2][0], ligthSpaceViewMatrices[i][2][1], ligthSpaceViewMatrices[i][2][2], ligthSpaceViewMatrices[i][2][3]);
    ImGui::Text("|%f|%f|%f|%f|", ligthSpaceViewMatrices[i][3][0], ligthSpaceViewMatrices[i][3][1], ligthSpaceViewMatrices[i][3][2], ligthSpaceViewMatrices[i][3][3]);
  }
  ImGui::Separator();
  ImGui::Text("Projection matrices:");
  ImGui::Text("|%f|%f|%f|%f|", lightSpaceProjection[0][0], lightSpaceProjection[0][1], lightSpaceProjection[0][2], lightSpaceProjection[0][3]);
  ImGui::Text("|%f|%f|%f|%f|", lightSpaceProjection[1][0], lightSpaceProjection[1][1], lightSpaceProjection[1][2], lightSpaceProjection[1][3]);
  ImGui::Text("|%f|%f|%f|%f|", lightSpaceProjection[2][0], lightSpaceProjection[2][1], lightSpaceProjection[2][2], lightSpaceProjection[2][3]);
  ImGui::Text("|%f|%f|%f|%f|", lightSpaceProjection[3][0], lightSpaceProjection[3][1], lightSpaceProjection[3][2], lightSpaceProjection[3][3]);*/
}

SpotLight::SpotLight(const std::string& name, glm::vec3 position, glm::vec3 direction, float innerfalloff, float outerfalloff, float constant, float linear, float quadratic, glm::vec3 color, float intensity, bool castshadow, bool transShadow, bool colorShadow, unsigned int shadowResolution, LightMobility mobility) :
  Light(LT_Spotlight, name, color, intensity, castshadow, transShadow, colorShadow, shadowResolution, mobility), position(position), direction(direction),
  innerFalloff(innerfalloff), outerFalloff(outerfalloff), constant(constant), linear(linear), quadratic(quadratic), nearPlane(0.1f), farPlane(7.5f)
{

}

SpotLight::~SpotLight() {}

void SpotLight::UpdateUniforms(int shaderID, int id)
{
  glUseProgram(shaderID);

  std::string arrayStr = "spotLights[" + std::to_string(id) + "].";

  GLuint loc = glGetUniformLocation(shaderID, (arrayStr + "position").c_str());
  glUniform3fv(loc, 1, &position[0]);
  loc = glGetUniformLocation(shaderID, (arrayStr + "direction").c_str());
  glUniform3fv(loc, 1, &direction[0]);
  loc = glGetUniformLocation(shaderID, (arrayStr + "diffuse").c_str());
  glUniform3fv(loc, 1, &GetDiffuseColor()[0]);
  loc = glGetUniformLocation(shaderID, (arrayStr + "ambient").c_str());
  glUniform3fv(loc, 1, &GetAmbientColor()[0]);
  loc = glGetUniformLocation(shaderID, (arrayStr + "specular").c_str());
  glUniform3fv(loc, 1, &GetSpecularColor()[0]);
  loc = glGetUniformLocation(shaderID, (arrayStr + "innerOff").c_str());
  glUniform1f(loc, glm::cos(glm::radians(innerFalloff)));
  loc = glGetUniformLocation(shaderID, (arrayStr + "outerOff").c_str());
  glUniform1f(loc, glm::cos(glm::radians(outerFalloff)));
  loc = glGetUniformLocation(shaderID, (arrayStr + "constant").c_str());
  glUniform1f(loc, constant);
  loc = glGetUniformLocation(shaderID, (arrayStr + "linear").c_str());
  glUniform1f(loc, linear);
  loc = glGetUniformLocation(shaderID, (arrayStr + "quadratic").c_str());
  glUniform1f(loc, quadratic);
}

void SpotLight::CalculateLightSpaceMatrix(Camera* camera)
{
  glm::mat4 lightProjection = glm::perspective(glm::radians(outerFalloff), 1.f, nearPlane, farPlane);
  glm::mat4 lightView = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
  //GetLightSpaceMatrix() = lightProjection * lightView;
}

bool SpotLight::FrustumCull(const BoundingVolume& bd) const
{
  return true;
}

void SpotLight::DrawDebugMenu()
{
  Light::DrawDebugMenu();

  ImGui::InputFloat3("Position", &position[0]);
  ImGui::InputFloat3("Direction", &direction[0]);
  ImGui::InputFloat("Inner Falloff", &innerFalloff);
  ImGui::InputFloat("Outer Falloff", &outerFalloff);
  ImGui::InputFloat("Constant", &constant);
  ImGui::InputFloat("Linear", &linear);
  ImGui::InputFloat("Quadratic", &quadratic);
}
