#include "Renderer.h"
#include "ResourceManager.h"
#include "Light.h"
#include "UniformBuffer.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "TextureAtlas.h"
#include "Window.h"
#include "Editor.h"
#include "EngineCore.h"
#include "Coordinator.h"
#include "Model.h"
#include "MeshSystem.h"
#include "SkinnedMeshSystem.h"
#include "BSpline.h"
#include "ClothSystem.h"
#include "Profiler.h"
#include "GraphicAPI.h"
#include "ShaderParser.h"
#include "CollisionSolver.h"
#include "Cubemap.h"
#include "MathUtil.h"
#include "TextureArray.h"
#include "TextureLoader.h"

#define MAXLIGHTNUM 16
#define DIRLIGHTSIZE (16*2+16*4+4)
#define POINTLIGHTSIZE (16*5)
#define SPOTLIGHTSIZE (16*5+4*5+16+4*3+4)

#define MAX_SHADOW_RESOLUTION 2048
#define MAX_SHADOW_DOWNSAMPLE 8

ShaderConfig::PolygonMode Renderer::polygonMode;

bool Renderer::FrustrumCulling = true;
bool Renderer::MSAA = false;

unsigned int Renderer::maxRenderObj;
unsigned int Renderer::currRenderObj;
std::vector<RenderObject> Renderer::renderObjs;

unsigned int Renderer::maxTransparentObj;
unsigned int Renderer::currTransparentObjCount;
std::vector<RenderObject> Renderer::transparentObjs;

Light* Renderer::EditLight;
RenderObject* Renderer::EditObj;
Camera* Renderer::EditCam;

std::shared_ptr<MeshSystem> Renderer::meshSystem;
std::shared_ptr<SkinnedMeshSystem> Renderer::skinnedMeshSystem;
std::shared_ptr<ClothSystem> Renderer::clothSystem;

unsigned Renderer::width, Renderer::height;
Rect<unsigned int> Renderer::viewRect;

glm::vec3 Renderer::clearColor;

ShadingTechnique Renderer::shadingTechnique;
bool Renderer::DrawMaterial;

unsigned Renderer::maxLights;
unsigned int Renderer::prevPointLightCount;
float Renderer::maxLightDistance = 100.f;
float Renderer::minLightDistance = 10.f;
float Renderer::maxLightUpdateRate = 0.6f;
float Renderer::minLightUpdateRate = 0.03f;
std::vector<PointLight*> Renderer::pointLights;
std::vector<SpotLight*> Renderer::spotLights;
std::vector<DirectionalLight*> Renderer::directionalLights;
bool Renderer::showLightIcon;
bool Renderer::showLightVolume;

std::vector<DebugObject> Renderer::debugObjects;
std::vector<DebugModel> Renderer::debugModels;
unsigned Renderer::debugObjSize;
Model* Renderer::debugMeshes[DOT_COUNT];

std::vector<BSpline*> Renderer::debugSplines;

bool Renderer::showBB;

Camera* Renderer::camera;

UniformBuffer* Renderer::UBmatrices;
UniformBuffer* Renderer::UBlights;
Shader* Renderer::TextureQuadShader;
Shader* Renderer::TextureQuadDepthShader;
Shader* Renderer::DefaultShader;
Shader* Renderer::DrawShadowShader;
Shader* Renderer::DrawCubeShadowShader;
Shader* Renderer::DrawTransparentShadowShader;
Shader* Renderer::DepthmapShader;
Shader* Renderer::gBufferShader;
Shader* Renderer::gBufferSkinnedShader;
Shader* Renderer::gBufferPreviewShaders[GBC_COUNT];
Shader* Renderer::lightPassShader;
Shader* Renderer::OITCombineShader;
Shader* Renderer::OITCopyOpaqueShader;
Shader* Renderer::DepthQuadShader;
Shader* Renderer::GaussianFilterShader;
Shader* Renderer::Gaussian5x5Shader;
Shader* Renderer::Gaussian9x9Shader;
Shader* Renderer::ConvertStochasticVSM;
Shader* Renderer::VSMCopyShader;
Shader* Renderer::BloomFilter;
Shader* Renderer::BloomCombine;
Shader* Renderer::SSAOShader;
Shader* Renderer::SSAOBlurShader;
Shader* Renderer::KawaseBlurUp;
Shader* Renderer::KawaseBlurDown;
Shader* Renderer::MomentShadow;
Shader* Renderer::MomentBlur;
TextureAtlas* Renderer::ShadowAtlas;
Framebuffer* Renderer::gBuffer;
Framebuffer* Renderer::finalOutput;
Framebuffer* Renderer::depthBuffer;
Framebuffer* Renderer::renderBuffer;
Framebuffer* Renderer::OpaqueBuffer;
Framebuffer* Renderer::OTIBuffer;
Framebuffer* Renderer::PingpongBuffer[2];
Framebuffer* Renderer::SSAOBuffer, * Renderer::SSAOBlur;

int Renderer::currentGBufferChannel;
bool Renderer::copyDepthInfo;
Model* Renderer::quad;

TextureAtlas* Renderer::textureAtlas;

glm::vec3 Renderer::FogColor;
glm::vec3 Renderer::GlobalAmbient;
glm::vec2 Renderer::FarPlane = glm::vec2(100.f, 500.f);

DebugObject Renderer::worldGrid[16];
bool Renderer::showGrid;

unsigned int Renderer::ClusterY = 9;
unsigned int Renderer::ClusterX = 16;
unsigned int Renderer::ClusterZ = 24;
unsigned int Renderer::numClusters = ClusterX * ClusterY * ClusterZ;
unsigned int Renderer::sizeX, Renderer::sizeY;

unsigned int Renderer::screenToViewSSBO;
unsigned int Renderer::AABBvolumeGridSSBO;
unsigned int Renderer::lightSSBO;
unsigned int Renderer::lightIndexListSSBO;
unsigned int Renderer::lightGridSSBO;
unsigned int Renderer::lightIndexGlobalCountSSBO;
unsigned int Renderer::activeClusterSSBO;
unsigned int Renderer::compactClusterSSBO;
unsigned int Renderer::globalCompactCountSSBO;
unsigned int Renderer::maxLightsPerTile = 50;
bool Renderer::ComputeClustersOnGPU = true;

ScreenToView Renderer::screen2View;
ComputeShader* Renderer::buildAABBGridCompShader;
ComputeShader* Renderer::filterClusterCompShader;
ComputeShader* Renderer::filterCompactCompShader;
ComputeShader* Renderer::cullLightsCompShader;
Shader* Renderer::cluserVisualShader;
Shader* Renderer::ClusterPhongShader;
Shader* Renderer::ClusterPhongAlphaShader;
struct GPULight* Renderer::lights;

float Renderer::VSMBlurAmount = 1.f;
float Renderer::VSMMin = 0.002f;
float Renderer::VSMLightBleedReduction = 0.2f;
glm::vec2 Renderer::MSMBleedReductDarken = glm::vec2(0.1f, 0.f);

Skybox *Renderer::skyBox;
Texture* Renderer::uniformNoise;;

int Renderer::bloomDownsample = 4;
float Renderer::bloomThreshold = 1.0f;
float Renderer::bloomRadius = 1.0f;
float Renderer::bloomIntensity = 0.2f;
int Renderer::bloomOffset = 3;
int Renderer::bloomIterations = 4;

std::vector<glm::vec3> Renderer::ssaoKernel;
Texture* Renderer::ssaoNoise;
float Renderer::ssaoRadius = 1.f;
float Renderer::ssaoBias = 0.025f;
float Renderer::ssaoBlur = 1.f;
float Renderer::ssaoPower = 3.f;

float Renderer::time = 0.f;
int Renderer::temporalIndex = 0;
int Renderer::temporalPrevIndex = 1;
int Renderer::temporalMax = 2;

bool Renderer::debugCascade = false;
int Renderer::cascadePartition = 3;
int Renderer::cascadeTransparentPartition = 2;
int Renderer::cascadeResolutions = 2048;
int Renderer::stochasticUpsample = 2;
int Renderer::stochasticDownsample = 2;
float Renderer::cascadeBlendWidth = 0.001f;
std::vector<float> Renderer::cascadeDistances;
std::vector<int> Renderer::cascadeUpdateRates;
std::vector<int> Renderer::cascadeUpdateTimers;
std::vector<FrustrumSphere> Renderer::cascadeSpheres;
std::vector<unsigned int> Renderer::ShadowResolutions;
std::vector<Framebuffer*> Renderer::ShadowBufferPool;
std::vector<Framebuffer*> Renderer::VSMBufferPool;
std::vector<Framebuffer*> Renderer::VSMBlurBufferPool;
std::vector<Framebuffer*> Renderer::VSMFinalBufferPool;
TextureArray* Renderer::CascadeTextures;
Texture* Renderer::CascadeStochasticTextures;
TextureArray* Renderer::CascadeVSMTextures;
Framebuffer* Renderer::CascadeBuffer;
Framebuffer* Renderer::CascadeBlurBuffer;
Framebuffer* Renderer::CascadeStochasticBuffer;
Framebuffer* Renderer::CascadeVSMBlurBuffer;
Framebuffer* Renderer::CascadeVSMFinalBuffer;

CubeMap* Renderer::IrradianceMap;
CubeMap* Renderer::PrefilterMap;
Texture* Renderer::BRDF;
float Renderer::prefilterMip = 1.f;

float Renderer::ssrMaxDist = 5;
float Renderer::ssrResolution = 0.3;
float Renderer::ssrSteps = 5;
float Renderer::ssrThickness =  0.5;
Framebuffer* Renderer::SSRBuffer;
Shader* Renderer::SSRShader;

Shader* Renderer::ColorCorrectionShader;
float Renderer::exposure = 3.f;

SH::SH_coeff Renderer::SphericalHarmonics;

void Renderer::Init(std::shared_ptr<MeshSystem> _meshSystem, std::shared_ptr<SkinnedMeshSystem> _skinnedMeshSystem, unsigned _width, unsigned _height, unsigned maxlights, ShadingTechnique shadingtechnique)
{
  LOG_TRACE_S("Renderer", "Initialize with width = {}, height = {}, maxlight = {}.", _width, _height, maxlights);

  ASSERT(_meshSystem != nullptr, "[Renderer] Mesh system is nullptr!");
  ASSERT(_skinnedMeshSystem != nullptr, "[Renderer] Skinned mesh system is nullptr!");
  
  meshSystem = _meshSystem;
  skinnedMeshSystem = _skinnedMeshSystem;

  time = 0.f;
  temporalIndex = 0;
  temporalPrevIndex = temporalMax - 1;

  width = _width; height = _height;

  clearColor = glm::vec3(0.1f, 0.1f, 0.1f);

  shadingTechnique = shadingtechnique;
  DrawMaterial = false;

  maxLights = maxlights;
  pointLights.reserve(maxLights);
  spotLights.reserve(maxLights);
  directionalLights.reserve(maxLights);

  debugObjSize = 0;
  debugObjects = std::vector<DebugObject>(10000, DebugObject());

  maxRenderObj = 10000;
  currRenderObj = 0;
  renderObjs.reserve(maxRenderObj);
  for (unsigned int i = 0; i < maxRenderObj; i++)
  {
    renderObjs.push_back(RenderObject());
  }

  maxTransparentObj = 100;
  currTransparentObjCount = 0;
  transparentObjs.reserve(maxTransparentObj);
  for (unsigned int i = 0; i < maxTransparentObj; i++)
  {
    transparentObjs.push_back(RenderObject());
  }

  uniformNoise = ResourceManager::Get<Texture>("uniform noise");

  UBmatrices = new UniformBuffer(sizeof(glm::mat4) * 3 + sizeof(glm::vec4) * 3 + sizeof(glm::vec2) * 3, 0);
  UBlights = new UniformBuffer(4 * 4 + (DIRLIGHTSIZE * MAXLIGHTNUM), 7);

  TextureQuadShader = ResourceManager::Get<Shader>("Texture Quad");
  TextureQuadDepthShader = ResourceManager::Get<Shader>("TextureQuadDepth");
  DefaultShader = ResourceManager::Get<Shader>("Default");
  DrawShadowShader = ResourceManager::Get<Shader>("Draw Shadow");
  DrawCubeShadowShader = ResourceManager::Get<Shader>("Draw Cube Shadow");
  DrawTransparentShadowShader = ResourceManager::Get<Shader>("Draw Transparent Shadow");
  DepthmapShader = ResourceManager::Get<Shader>("Depth Map");

#ifdef DEFERRED_SHADING
  gBufferShader = ResourceManager::Get<Shader>("Clustered Geometry");
  lightPassShader = ResourceManager::Get<Shader>("Lightpass PBR");
  ClusterPhongAlphaShader = ResourceManager::Get<Shader>("Cluster PBR Alpha");
#else
  cluserVisualShader = ResourceManager::Get<Shader>("Cluster visual");
  ClusterPhongShader = ResourceManager::Get<Shader>("Cluster Phong");
  ClusterPhongAlphaShader = ResourceManager::Get<Shader>("Cluster Phong Alpha");
#endif

  OITCopyOpaqueShader = ResourceManager::Get<Shader>("OIT Copy Opaque");
  OITCombineShader = ResourceManager::Get<Shader>("Combine OIT");
  DepthQuadShader = ResourceManager::Get<Shader>("Depth Quad");
  GaussianFilterShader = ResourceManager::Get<Shader>("Gaussian Filter");
  ConvertStochasticVSM = ResourceManager::Get<Shader>("Stochastic to VSM");
  VSMCopyShader = ResourceManager::Get<Shader>("VSM Copy");

  Gaussian5x5Shader = ResourceManager::Get<Shader>("Gaussian 5x5");
  Gaussian9x9Shader = ResourceManager::Get<Shader>("Gaussian 9x9");
  KawaseBlurUp = ResourceManager::Get<Shader>("Kawase Up");
  KawaseBlurDown = ResourceManager::Get<Shader>("Kawase Down");
  BloomFilter = ResourceManager::Get<Shader>("Bloom Filter");
  BloomCombine = ResourceManager::Get<Shader>("Bloom Combine");

  SSAOShader = ResourceManager::Get<Shader>("SSAO");
  SSAOBlurShader = ResourceManager::Get<Shader>("SSAO Blur");

  SSRShader = ResourceManager::Get<Shader>("SSR");

  ColorCorrectionShader = ResourceManager::Get<Shader>("Color Correction");

  MomentShadow = ResourceManager::Get<Shader>("Moment Shadow");
  MomentBlur = ResourceManager::Get<Shader>("Moment Blur");

  depthBuffer = new Framebuffer(0, 0, width, height, true);
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  Texture* prepassMap = new Texture(nullptr, depthBuffer->GetWidht(), depthBuffer->GetHeight(), TT_2D, 4, TEB_Wrap, TAB_Nearest, TC_Depth, TC_Depth, DT_Float);
  prepassMap->SetBorderColor(borderColor);
  depthBuffer->AddBufferTexture("depth map", prepassMap, BA_Depth, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer->GetFBO());
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifdef DEFERRED_SHADING
  gBuffer = new Framebuffer(0, 0, width, height, true);
  gBuffer->AddBufferTexture("position", new Texture(nullptr, width, height, TT_2D, 4, TEB_Clamp, TAB_Nearest, TC_RGB, TC_RGB16F, DT_Float), BA_Color, 0);
  gBuffer->AddBufferTexture("normal", new Texture(nullptr, width, height, TT_2D, 4, TEB_Clamp, TAB_Nearest, TC_RGB, TC_RGB16F, DT_Float), BA_Color, 1);
  gBuffer->AddBufferTexture("diffuse", new Texture(nullptr, width, height, TT_2D, 4, TEB_Clamp, TAB_Nearest, TC_RGBA, TC_RGBA, DT_uByte), BA_Color, 2);
  gBuffer->AddBufferTexture("RMA", new Texture(nullptr, width, height, TT_2D, 4, TEB_Clamp, TAB_Nearest, TC_RGB, TC_RGB, DT_uByte), BA_Color, 3);
  gBuffer->AddBufferTexture("F0", new Texture(nullptr, width, height, TT_2D, 4, TEB_Clamp, TAB_Nearest, TC_RGB, TC_RGB, DT_uByte), BA_Color, 4);
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer->GetFBO());
  unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
  glDrawBuffers(5, attachments);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  gBufferPreviewShaders[GBC_Position] = ResourceManager::Get<Shader>("G-buffer Position");
  gBufferPreviewShaders[GBC_Normal] = ResourceManager::Get<Shader>("G-buffer Normal");
  gBufferPreviewShaders[GBC_Ambient] = ResourceManager::Get<Shader>("G-buffer Ambient");
  gBufferPreviewShaders[GBC_Diffuse] = ResourceManager::Get<Shader>("G-buffer Diffuse");
  gBufferPreviewShaders[GBC_Specular] = ResourceManager::Get<Shader>("G-buffer Specular");
  gBufferPreviewShaders[GBC_Shininess] = ResourceManager::Get<Shader>("G-buffer Shininess");
  gBufferPreviewShaders[GBC_Shadow] = ResourceManager::Get<Shader>("G-buffer Shadow");
#endif

#ifdef MSAA_ON
  glEnable(GL_MULTISAMPLE);

  renderBuffer = new Framebuffer(posX, posY, width, height, true, true);
  renderBuffer->AddBufferTexture("color", new Texture(nullptr, gBuffer->GetWidht(), gBuffer->GetHeight(), TT_2DMSAA, 4, TEB_Wrap, TAB_Nearest, TC_RGB, TC_RGB16F, DT_Float), BA_Color, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer->GetFBO());
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  finalOutput = new Framebuffer(posX, posY, width, height, false);
  finalOutput->AddBufferTexture("color", new Texture(nullptr, gBuffer->GetWidht(), gBuffer->GetHeight(), TT_2D, 4, TEB_Wrap, TAB_Nearest, TC_RGB, TC_RGB16F, DT_Float), BA_Color, 0);
#else
  OpaqueBuffer = new Framebuffer(0, 0, width, height, true, false);
  OpaqueBuffer->AddBufferTexture("color", new Texture(nullptr, width, height, TT_2D, 4, TEB_Wrap, TAB_Nearest, TC_RGB, TC_RGB16F, DT_Float), BA_Color, 0);
  OpaqueBuffer->AddBufferTexture("depth", new Texture(nullptr, width, height, TT_2D, 1, TEB_Wrap, TAB_Nearest, TC_Depth, TC_Depth16, DT_Float), BA_Depth, 1);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  renderBuffer = new Framebuffer(0, 0, width, height, true, false);
  renderBuffer->AddBufferTexture("color", new Texture(nullptr, width, height, TT_2D, 4, TEB_Wrap, TAB_Nearest, TC_RGB, TC_RGB16F, DT_Float), BA_Color, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer->GetFBO());
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  finalOutput = new Framebuffer(0, 0, width, height, false);
  finalOutput->AddBufferTexture("color", new Texture(nullptr, width, height, TT_2D, 4, TEB_Wrap, TAB_Nearest, TC_RGB, TC_RGB, DT_uByte), BA_Color, 0);
#endif

  //GenShadowBufferPools();
  float cascadeDistStep = 1.f / (cascadePartition + 1);
  for (int i = 0; i < cascadePartition; i++)
  {
    cascadeDistances.push_back(cascadeDistStep * (i + 1));
    cascadeSpheres.push_back(FrustrumSphere());
    cascadeUpdateRates.push_back(0);
    cascadeUpdateTimers.push_back(0);
  }
  cascadeDistances[0] = 0.025f;
  cascadeDistances[1] = 0.08f;
  cascadeDistances[2] = 0.2f;
  cascadeUpdateRates[0] = 0;
  cascadeUpdateRates[1] = 5;
  cascadeUpdateRates[2] = 15;
  for (int i = 0; i < cascadePartition; i++)
  {
    float nearDist, farDist;
    glm::vec3 newPos;
    UpdateCascadeCenter(i, nearDist, farDist, newPos);

    cascadeSpheres[i].center = newPos;
  }

  CascadeBuffer = new Framebuffer(0, 0, cascadeResolutions, cascadeResolutions);
  CascadeTextures = new TextureArray(cascadePartition + 1, cascadeResolutions, cascadeResolutions, TT_2D_ARRAY, TC_Depth, TC_Depth16, DT_Float, TEB_Clamp, TAB_Nearest);
  float cascadeBorder[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  CascadeTextures->SetBorderColor(cascadeBorder);
  CascadeBuffer->AddBufferArrayTexture("shadow", CascadeTextures, BufferAttachment::BA_Depth, 0, 0);
  //CascadeBuffer->SetClearColor(glm::vec4(1.f));
  //CascadeBuffer->SetClearDepth(1.f);

  //CascadeBlurBuffer = new Framebuffer(0, 0, cascadeResolutions, cascadeResolutions);
  //CascadeBlurBuffer->AddBufferArrayTexture("moments", CascadeTextures, BufferAttachment::BA_Color, 0, cascadePartition);

#ifdef TRANSPARENT_SHADOW
  CascadeStochasticBuffer = new Framebuffer(0, 0, cascadeResolutions * stochasticUpsample, cascadeResolutions * stochasticUpsample);
  CascadeVSMBlurBuffer = new Framebuffer(0, 0, cascadeResolutions / stochasticDownsample, cascadeResolutions / stochasticDownsample);
  CascadeVSMFinalBuffer = new Framebuffer(0, 0, cascadeResolutions / stochasticDownsample, cascadeResolutions / stochasticDownsample);

#ifdef COLOR_STOCHASTIC_SHADOW
  // *2 for mean and variance, +2 for blur tmp
  CascadeVSMTextures = new TextureArray(cascadeTransparentPartition * 2 + 2, cascadeResolutions / stochasticDownsample, cascadeResolutions / stochasticDownsample, TT_2D_ARRAY, TC_RGB, TC_RGB, DT_Float, TEB_Clamp, TAB_LinearMipmap);

  CascadeStochasticBuffer->AddBufferTexture("mean", new Texture(nullptr, cascadeResolutions* stochasticUpsample, cascadeResolutions* stochasticUpsample, TT_2D, 4, TEB_Clamp, TAB_Linear, TC_RGB, TC_RGB, DT_Float), BufferAttachment::BA_Color, 0);
  CascadeStochasticBuffer->AddBufferTexture("variance", new Texture(nullptr, cascadeResolutions* stochasticUpsample, cascadeResolutions* stochasticUpsample, TT_2D, 4, TEB_Clamp, TAB_Linear, TC_RGB, TC_RGB, DT_Float), BufferAttachment::BA_Color, 1);

  CascadeVSMBlurBuffer->AddBufferArrayTexture("mean", CascadeVSMTextures, BufferAttachment::BA_Color, 0, cascadeTransparentPartition * 2);
  CascadeVSMBlurBuffer->AddBufferArrayTexture("variance", CascadeVSMTextures, BufferAttachment::BA_Color, 1, cascadeTransparentPartition * 2 + 1);

  CascadeVSMFinalBuffer->AddBufferArrayTexture("mean", CascadeVSMTextures, BufferAttachment::BA_Color, 0, 0);
  CascadeVSMFinalBuffer->AddBufferArrayTexture("variance", CascadeVSMTextures, BufferAttachment::BA_Color, 1, 1);
#else

  CascadeVSMTextures = new TextureArray(cascadeTransparentPartition + 1, cascadeResolutions / stochasticDownsample, cascadeResolutions / stochasticDownsample, TT_2D_ARRAY, TC_RG, TC_RG, DT_Float, TEB_Clamp, TAB_LinearMipmap);

  CascadeStochasticBuffer->AddBufferTexture("shadow", new Texture(nullptr, cascadeResolutions* stochasticUpsample, cascadeResolutions* stochasticUpsample, TT_2D, 4, TEB_Clamp, TAB_Linear, TC_RG, TC_RG, DT_Float), BufferAttachment::BA_Color, 0);
  CascadeVSMBlurBuffer->AddBufferArrayTexture("shadow", CascadeVSMTextures, BufferAttachment::BA_Color, 0, 0);
  CascadeVSMFinalBuffer->AddBufferArrayTexture("shadow", CascadeVSMTextures, BufferAttachment::BA_Color, 0, 1);
#endif
#endif

  OTIBuffer = new Framebuffer(0, 0, width, height, true);
  OTIBuffer->AddBufferTexture("accum", new Texture(nullptr, width, height, TT_2D, 4, TEB_Clamp, TAB_Nearest, TC_RGBA, TC_RGBA16F, DT_Float), BA_Color, 0);
  OTIBuffer->AddBufferTexture("beta", new Texture(nullptr, width, height, TT_2D, 1, TEB_Clamp, TAB_Nearest, TC_RGBA, TC_RGBA8, DT_uByte), BA_Color, 1);
  OTIBuffer->AddBufferTexture("refract", new Texture(nullptr, width, height, TT_2D, 1, TEB_Clamp, TAB_Nearest, TC_RG, TC_RG8SNORM, DT_uByte), BA_Color, 2);
  glBindFramebuffer(GL_FRAMEBUFFER, OTIBuffer->GetFBO());
  unsigned int oitAttach[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  glDrawBuffers(3, oitAttach);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  unsigned int pingpongX = width / bloomDownsample, pingpongY = height/ bloomDownsample;
  PingpongBuffer[0] = new Framebuffer(0, 0, pingpongX, pingpongY, false);
  PingpongBuffer[0]->AddBufferTexture("color", new Texture(nullptr, pingpongX, pingpongY, TT_2D, 4, TEB_Clamp, TAB_Linear, TC_RGB, TC_RGB16F, DT_Float), BA_Color, 0);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  PingpongBuffer[1] = new Framebuffer(0, 0, pingpongX, pingpongY, false);
  PingpongBuffer[1]->AddBufferTexture("color", new Texture(nullptr, pingpongX, pingpongY, TT_2D, 4, TEB_Clamp, TAB_Linear, TC_RGB, TC_RGB16F, DT_Float), BA_Color, 0);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  int ssaoDownsample = 2;
  SSAOBuffer = new Framebuffer(0, 0, width / ssaoDownsample, height / ssaoDownsample);
  SSAOBuffer->AddBufferTexture("color", new Texture(nullptr, width / ssaoDownsample, height / ssaoDownsample, TT_2D, 4, TEB_Clamp, TAB_Linear, TC_RED, TC_R32F, DT_Float), BA_Color, 0);
  SSAOBlur = new Framebuffer(0, 0, width / 2, height / 2);
  SSAOBlur->AddBufferTexture("color", new Texture(nullptr, width / 2, height / 2, TT_2D, 4, TEB_Clamp, TAB_Linear, TC_RED, TC_R32F, DT_Float), BA_Color, 0);

  // generate ssao kernel
  for (unsigned int i = 0; i < 64; ++i)
  {
    glm::vec3 sample(MathUtil::RandomRange(0.f, 1.f) * 2.0 - 1.0, MathUtil::RandomRange(0.f, 1.f)* 2.0 - 1.0, MathUtil::RandomRange(0.f, 1.f));
    sample = glm::normalize(sample);
    sample *= MathUtil::RandomRange(0.f, 1.f);
    float scale = float(i) / 64.0;

    // scale samples s.t. they're more aligned to center of kernel
    scale = glm::mix(0.1f, 1.0f, scale * scale);
    sample *= scale;
    ssaoKernel.push_back(sample);
  }
  std::vector<glm::vec3> ssaoNoiseData;
  for (unsigned int i = 0; i < 16; i++)
  {
    glm::vec3 noise(MathUtil::RandomRange(0.f, 1.f)* 2.0 - 1.0, MathUtil::RandomRange(0.f, 1.f)* 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
    ssaoNoiseData.push_back(noise);
  }
  ssaoNoise = new Texture(ssaoNoiseData, 4, 4, TT_2D, TC_RGB, TC_RGB, DT_Float, TEB_Wrap, TAB_Nearest);

  int ssrDownsample = 2;
  SSRBuffer = new Framebuffer(0, 0, width / ssrDownsample, height / ssrDownsample);
  SSRBuffer->AddBufferTexture("color", new Texture(nullptr, width / ssrDownsample, height / ssrDownsample, TT_2D, 4, TEB_Clamp, TAB_Linear, TC_RGB, TC_RGB, DT_Float), BA_Color, 0);

  quad = ResourceManager::GetQuad();
  LoadDebugMeshes();

  /*textureAtlas = new TextureAtlas(ResourceManager::Get<Shader>("texture atlas"), 4096, 8, TC_RGBA, TC_RGBA, DT_uByte);
  textureAtlas->WriteToTexture(ResourceManager::Get<Texture>("dirt_diff"), 0);
  textureAtlas->WriteToTexture(ResourceManager::Get<Texture>("dirt_norm"), 1);
  textureAtlas->WriteToTexture(ResourceManager::Get<Texture>("dirt_spec"), 2);
  textureAtlas->WriteToTexture(ResourceManager::Get<Texture>("stone_diff"), 3);
  textureAtlas->WriteToTexture(ResourceManager::Get<Texture>("stone_norm"), 4);
  textureAtlas->WriteToTexture(ResourceManager::Get<Texture>("stone_spec"), 5);
  textureAtlas->WriteToTexture(ResourceManager::Get<Texture>("cliff_diff"), 6);
  textureAtlas->WriteToTexture(ResourceManager::Get<Texture>("cliff_norm"), 7);
  textureAtlas->WriteToTexture(ResourceManager::Get<Texture>("cliff_spec"), 8);
  textureAtlas->WriteToTexture(ResourceManager::Get<Texture>("rock_diff"), 9);
  textureAtlas->WriteToTexture(ResourceManager::Get<Texture>("rock_norm"), 10);
  textureAtlas->WriteToTexture(ResourceManager::Get<Texture>("rock_spec"), 11);*/

  for (int z = -2; z < 2; z++)
  {
    for (int x = -2; x < 2; x++)
    {
      Transform transform(glm::vec3(x + 0.5f, 0.f, z + 0.5f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(2.f), 0.f);
      transform.UpdateMatrix();
      worldGrid[(z + 2) * 4 + (x + 2)].Init(DOT_LineSquare, transform, glm::vec3(0.6f));
    }
  }

  showLightIcon = true;
  showLightVolume = false;
  showGrid = true;
  copyDepthInfo = true;
  showBB = false;

  Renderer::SetPolygonMode(ShaderConfig::Fill);
  currentGBufferChannel = 0;
  
  FogColor = glm::vec3(0.7f, 0.7f, 0.8f);
  GlobalAmbient = glm::vec3(0.177f, 0.221f, 0.297f);

  skyBox = new Skybox(ResourceManager::Get<Material>("Skybox Cubemap"));
  IrradianceMap = ResourceManager::Get<CubeMap>("irridiance map");
  PrefilterMap = ResourceManager::Get<CubeMap>("prefilter map");
  BRDF = ResourceManager::Get<Texture>("BRDF");

  buildAABBGridCompShader = new ComputeShader("../Common/shaders/Compute/CalcCluster.comp");
  cullLightsCompShader = new ComputeShader("../Common/shaders/Compute/CullLight.comp");
  filterClusterCompShader = new ComputeShader("../Common/shaders/Compute/FilterCluster.comp");
  filterCompactCompShader = new ComputeShader("../Common/shaders/Compute/CompactCluster.comp");

  InitSSBOs();
  CalculateClusters();

  prevPointLightCount = pointLights.size();
}

void Renderer::Exit()
{
  LOG_TRACE("Renderer", "Exitting renderer...");

  delete buildAABBGridCompShader;
  delete filterClusterCompShader;
  delete filterCompactCompShader;
  delete cullLightsCompShader;

  glDeleteBuffers(1, &AABBvolumeGridSSBO);
  glDeleteBuffers(1, &screenToViewSSBO);
  glDeleteBuffers(1, &activeClusterSSBO);
  glDeleteBuffers(1, &compactClusterSSBO);
  glDeleteBuffers(1, &globalCompactCountSSBO);
  glDeleteBuffers(1, &lightSSBO);
  glDeleteBuffers(1, &lightIndexListSSBO);
  glDeleteBuffers(1, &lightGridSSBO);
  glDeleteBuffers(1, &lightIndexGlobalCountSSBO);
  
  delete skyBox;

#ifdef DEFERRED_SHADING
  delete gBuffer;
#endif
  delete renderBuffer;
  delete depthBuffer;
  delete OTIBuffer;
  delete OpaqueBuffer;
  delete PingpongBuffer[0];
  delete PingpongBuffer[1];
  delete SSAOBuffer;
  delete SSAOBlur;
  delete ssaoNoise;
#ifdef MSAA_ON
  delete finalOutput;
#endif
  
  delete CascadeBuffer; // cascadeTextures get delete along
  //CascadeBlurBuffer->RemoveAllBufferTextures(); // prevent double delete
  //delete CascadeBlurBuffer;
  delete CascadeStochasticBuffer; // cascadeStochasticTextures get delete along

  delete CascadeVSMTextures;
  CascadeVSMBlurBuffer->RemoveAllBufferTextures(); // prevent double delete
  delete CascadeVSMBlurBuffer;
  CascadeVSMFinalBuffer->RemoveAllBufferTextures(); // prevent double delete since CascadeVSMBlurBuffer also use same texture array
  delete CascadeVSMFinalBuffer;
  /*for (int i = 0; i < MAX_SHADOW_DOWNSAMPLE; i++)
  {
    #ifdef TRANSPARENT_SHADOW
      delete ShadowBufferPool[i];
      delete VSMBufferPool[i];
      delete VSMBlurBufferPool[i];
      delete VSMFinalBufferPool[i];
    #else
      delete ShadowBufferPool[i];
    #endif
  }*/

  for (unsigned int i = 0; i < directionalLights.size(); i++)
    delete directionalLights[i];

  for (unsigned int i = 0; i < pointLights.size(); i++)
    delete pointLights[i];

  for (unsigned int i = 0; i < spotLights.size(); i++)
    delete spotLights[i];

  debugObjSize = 0;

  delete UBmatrices;
  delete UBlights;

  //delete ShadowAtlas;
  //delete VSMShadowAtlas;

  //delete textureAtlas;
}

void Renderer::GenShadowBufferPools()
{
  //ShadowAtlas = new TextureAtlas(ResourceManager::Get<Shader>("shadow atlas"), 8192, 2, TC_Depth, TC_Depth, DT_Float);
  //VSMShadowAtlas = new TextureAtlas(ResourceManager::Get<Shader>("shadow atlas"), 8192, 2, TC_RGB, TC_RGB, DT_Float);

  unsigned int shadowRes = MAX_SHADOW_RESOLUTION;
  for (int i = 0; i < MAX_SHADOW_DOWNSAMPLE; i++)
  {
    ShadowResolutions.push_back(shadowRes);

    #ifdef TRANSPARENT_SHADOW
      #ifdef COLOR_STOCHASTIC_SHADOW
        
        // opaque
        Framebuffer* shwBuffer = new Framebuffer(0, 0, shadowRes, shadowRes);
        glBindFramebuffer(GL_FRAMEBUFFER, shwBuffer->GetFBO());
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        Texture* shadowMap = new Texture(nullptr, shadowRes, shadowRes, TT_2D, 4, TEB_Wrap, TAB_Nearest, TC_Depth, TC_Depth, DT_Float);
        shadowMap->SetBorderColor(borderColor);
        shwBuffer->AddBufferTexture("depth map", shadowMap, BA_Depth, 0);

        // stochastic
        int upsample = 2;
        unsigned int vsmattachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        Framebuffer *VSMShadow = new Framebuffer(0, 0, shadowRes * upsample, shadowRes * upsample);
        glBindFramebuffer(GL_FRAMEBUFFER, VSMShadow->GetFBO());
        VSMShadow->AddBufferTexture("mean", new Texture(nullptr, shadowRes * upsample, shadowRes * upsample, TT_2D, 4, TEB_Clamp, TAB_Linear, TC_RGB, TC_RGB, DT_Float), BA_Color, 0);
        VSMShadow->AddBufferTexture("variance", new Texture(nullptr, shadowRes * upsample, shadowRes * upsample, TT_2D, 4, TEB_Clamp, TAB_Linear, TC_RGB, TC_RGB, DT_Float), BA_Color, 1);
        glDrawBuffers(2, vsmattachments);

        // blur vsm
        int downsample = 1;
        Framebuffer* VSMBlurShadow = new Framebuffer(0, 0, shadowRes / downsample, shadowRes / downsample);
        glBindFramebuffer(GL_FRAMEBUFFER, VSMBlurShadow->GetFBO());
        VSMBlurShadow->AddBufferTexture("mean", new Texture(nullptr, shadowRes / downsample, shadowRes / downsample, TT_2D, 4, TEB_Clamp, TAB_Linear, TC_RGB, TC_RGB, DT_Float), BA_Color, 0);
        VSMBlurShadow->AddBufferTexture("variance", new Texture(nullptr, shadowRes / downsample, shadowRes / downsample, TT_2D, 4, TEB_Clamp, TAB_Linear, TC_RGB, TC_RGB, DT_Float), BA_Color, 1);
        glDrawBuffers(2, vsmattachments);

        // final vsm
        Framebuffer* VSMFinalShadow = new Framebuffer(0, 0, shadowRes / downsample, shadowRes / downsample);
        glBindFramebuffer(GL_FRAMEBUFFER, VSMFinalShadow->GetFBO());
        VSMFinalShadow->AddBufferTexture("mean", new Texture(nullptr, shadowRes / downsample, shadowRes / downsample, TT_2D, 4, TEB_Clamp, TAB_LinearMipmap, TC_RGB, TC_RGB, DT_Float), BA_Color, 0);
        VSMFinalShadow->AddBufferTexture("variance", new Texture(nullptr, shadowRes / downsample, shadowRes / downsample, TT_2D, 4, TEB_Clamp, TAB_LinearMipmap, TC_RGB, TC_RGB, DT_Float), BA_Color, 1);
        glDrawBuffers(2, vsmattachments);

      #else

      #endif
    #else
      Framebuffer* shwBuffer = new Framebuffer(0, 0, shadowRes, shadowRes);
    #endif

    ShadowBufferPool.push_back(shwBuffer);
    VSMBufferPool.push_back(VSMShadow);
    VSMBlurBufferPool.push_back(VSMBlurShadow);
    VSMFinalBufferPool.push_back(VSMFinalShadow);
    shadowRes /= 2;
  }
}

void Renderer::InitSSBOs()
{
  // Setting up tile size on both X and Y 
  sizeX = (unsigned int)std::ceilf(width / (float)ClusterX);

  float zFar = camera->GetFar();
  float zNear = camera->GetNear();

  // NOTE: ssbo is ubo but larger (16kb vs 128mb), ssbo are writables, ssbo can have variable storage, ssbo have slower access

  // Buffer containing all the clusters
  {
    glGenBuffers(1, &AABBvolumeGridSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, AABBvolumeGridSSBO);

    // We generate the buffer but don't populate it yet.
    glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * sizeof(struct VolumeTileAABB), NULL, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, AABBvolumeGridSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  // Setting up screen2View ssbo
  {
    glGenBuffers(1, &screenToViewSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, screenToViewSSBO);

    // Setting up contents of buffer
    screen2View.inverseProjectionMat = camera->GetInvProjectionMatrix();
    screen2View.tileSizes[0] = ClusterX;
    screen2View.tileSizes[1] = ClusterY;
    screen2View.tileSizes[2] = ClusterZ;
    screen2View.tileSizes[3] = sizeX;
    screen2View.screenWidth = width;
    screen2View.screenHeight = height;
    // Basically reduced a log function into a simple multiplication an addition by pre-calculating these
    screen2View.sliceScalingFactor = (float)ClusterZ / std::log2f(zFar / zNear);
    screen2View.sliceBiasFactor = -((float)ClusterZ * std::log2f(zNear) / std::log2f(zFar / zNear));

    // Generating and copying data to memory in GPU
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(struct ScreenToView), &screen2View, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, screenToViewSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  // Setting up lights buffer that contains all the lights in the scene
  {
    glGenBuffers(1, &lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, maxLights * sizeof(struct GPULight), NULL, GL_DYNAMIC_DRAW);

    GLint bufMask = GL_READ_WRITE;

    lights = (struct GPULight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, bufMask);
    for (unsigned int i = 0; i < pointLights.size(); ++i)
    {
      // Fetching the light from the current scene
      lights[i].position = glm::vec4(pointLights[i]->GetPosition(), 1.0f);
      lights[i].color = glm::vec4(pointLights[i]->GetDiffuseColor(), 1.0f);
      lights[i].enabled = 1;
      lights[i].intensity = 1.0f;
      lights[i].range = pointLights[i]->GetRadius();
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  // A list of indices to the lights that are active and intersect with a cluster
  {
    unsigned int totalNumLights = numClusters * maxLightsPerTile; //50 lights per tile max
    glGenBuffers(1, &lightIndexListSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexListSSBO);

    // We generate the buffer but don't populate it yet.
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalNumLights * sizeof(unsigned int), NULL, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, lightIndexListSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  // Every tile takes two unsigned ints one to represent the number of lights in that grid
  // Another to represent the offset to the light index list from where to begin reading light indexes from
  // This implementation is straight up from Olsson paper
  {
    glGenBuffers(1, &lightGridSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightGridSSBO);

    glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * 2 * sizeof(unsigned int), NULL, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightGridSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  // Setting up simplest ssbo in the world
  {
    glGenBuffers(1, &lightIndexGlobalCountSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexGlobalCountSSBO);

    // Every tile takes two unsigned ints one to represent the number of lights in that grid
    // Another to represent the offset 
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), NULL, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, lightIndexGlobalCountSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  // Buffer of loose array of clusters mark active
  {
    glGenBuffers(1, &activeClusterSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, activeClusterSSBO);

    // We generate the buffer but don't populate it yet.
    glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * sizeof(unsigned int), NULL, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, activeClusterSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  // Buffer of compact array of active clusters
  {
    glGenBuffers(1, &compactClusterSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, compactClusterSSBO);

    // We generate the buffer but don't populate it yet.
    glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * sizeof(unsigned int), NULL, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, compactClusterSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  // Setting up simplest ssbo in the world
  {
    glGenBuffers(1, &globalCompactCountSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, globalCompactCountSSBO);

    // Every tile takes two unsigned ints one to represent the number of lights in that grid
    // Another to represent the offset 
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), NULL, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, globalCompactCountSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }
}

void Renderer::Draw()
{
#ifdef DEFERRED_SHADING
  DeferredShading();
#else
  ClusteredForward();
#endif

  time += Time::DT();
  temporalPrevIndex = temporalIndex;
  temporalIndex = (temporalIndex + 1 >= temporalMax) ? 0 : temporalIndex + 1;
}

void Renderer::UpdateGlobalUniforms()
{
  unsigned int offset = 0;
  camera->UpdateMatrix();
  UBmatrices->Bind();
  UBmatrices->setMat4(camera->GetFinalMatrix(), offset);
  UBmatrices->setMat4(camera->GetProjMatrix(), offset);
  UBmatrices->setMat4(camera->GetLookAtMatrix(), offset);
  UBmatrices->setVec3(camera->GetPosition(), offset);
  UBmatrices->setVec3(FogColor, offset);
  UBmatrices->setVec3(GlobalAmbient, offset);
  UBmatrices->setVec2(glm::vec2(camera->GetNear(), camera->GetFar()), offset);
  UBmatrices->setVec2(FarPlane, offset);
  UBmatrices->setVec2(glm::vec2((float)width, (float)height), offset);
}

void Renderer::UpdateLights()
{
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
  lights = (struct GPULight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);

  for (unsigned int i = 0; i < pointLights.size(); ++i)
  {
    // if light goes beyond max dist, replace it with 2d sprite

    // don't render shadow of light that doesn't in the view
    const BoundingVolume bv = pointLights[i]->GetBoundingVolume();
    if (pointLights[i]->IsCastShadow() && camera->GetFrustrum().FastCullSphere(bv.center, bv.radius) == true)
    {
      // update shadow update timer
      pointLights[i]->IncrShadowUpdateTimer(Time::DT());
      bool updateShadow = false;
      if (pointLights[i]->GetShadowUpdateTimer() >= pointLights[i]->GetShadowUpdateRate())
      {
        updateShadow = true;
        pointLights[i]->ResetShadowUpdateTimer();
        pointLights[i]->skippedShadowFrames = 0;
      }
      pointLights[i]->skippedShadowFrames++;
      // update point light update rate depending on distance from camera
      glm::vec3 diffFromCam = camera->GetPosition() - pointLights[i]->GetPosition();
      float distFromCamSqr = glm::abs(glm::dot(diffFromCam, diffFromCam));
      // 0.05 = fastest rate, 2.0 = slowest
      pointLights[i]->SetShadowUpdateRate(minLightUpdateRate + (maxLightUpdateRate - minLightUpdateRate) * glm::min(glm::max(distFromCamSqr - minLightDistance * minLightDistance, 0.f) / (maxLightDistance * maxLightDistance), 1.0f));

      // update render shadow if need to update
      if (updateShadow)
      {
        //DrawCubeShadowShader->use();
        //pointLights[i]->StartCaptureDepthMap(DrawCubeShadowShader, camera);
        //DrawCubeShadowShader->setMat4Array("shadowMatrices[0]", pointLights[i]->GetCubeSpaceMatrices());
        //DrawCubeShadowShader->setVec3("lightPos", pointLights[i]->GetPosition());
        //DrawCubeShadowShader->setFloat("far_plane", pointLights[i]->GetRadius());

        //for (unsigned int j = 0; j < currRenderObj; j++)
        //{
        //  // culling obj that are not in radius
        //  if (renderObjs[j].boundingVolume.GetType() == BoundingVolumeTypes::BVT_Sphere)
        //  {
        //    if (CollisionSolver::SphereSphereTest(renderObjs[j].boundingVolume, pointLights[i]->GetBoundingVolume()) == true)
        //    {
        //      renderObjs[j].Draw(DrawCubeShadowShader);
        //    }
        //  }
        //  else
        //  {
        //    if (CollisionSolver::BoxSphereTest(renderObjs[j].boundingVolume, pointLights[i]->GetBoundingVolume()) == true)
        //    {
        //      renderObjs[j].Draw(DrawCubeShadowShader);
        //    }
        //  }
        //}

        //pointLights[i]->EndCaptureDepthMap();
      }
    }

    // Fetching the light from the current scene
    lights[i].position = glm::vec4(pointLights[i]->GetPosition(), 1.0f);
    lights[i].color = glm::vec4(pointLights[i]->GetDiffuseColor(), 1.0f);
    lights[i].enabled = 1;
    lights[i].intensity = pointLights[i]->GetIntensity();
    lights[i].range = pointLights[i]->GetRadius();
    lights[i].haveShadow = pointLights[i]->IsCastShadow();
  }

  // disable any removed point lights
  for (unsigned int i = pointLights.size(); i < prevPointLightCount; i++)
  {
    lights[i].enabled = 0;
  }
  prevPointLightCount = pointLights.size();

  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  unsigned int offset = 0;
  UBlights->Bind();
  unsigned dirLightNum = 0;
  unsigned castShadowNum = 0;
  for (unsigned i = 0; i < directionalLights.size(); i++)
  {
    if (directionalLights[i]->IsDisable()) continue;

    offset = 4 * 4 + DIRLIGHTSIZE * dirLightNum;

    UBlights->setVec4(glm::vec4(glm::normalize(directionalLights[i]->GetDirection()), 1.f), offset);
    UBlights->setVec4(glm::vec4(directionalLights[i]->GetDiffuseColor(), 1.f), offset);

    UBlights->setMat4(directionalLights[i]->GetLightSpaceMatrix(0), offset);
    UBlights->setBool(directionalLights[i]->IsCastShadow(), offset);
    UBlights->setFloat(directionalLights[i]->GetBias(), offset);
    if (directionalLights[i]->IsCastShadow())
    {
      // OPAQUE PASS
      // disable color write, enable depth write

      directionalLights[i]->CalculateLightSpaceMatrix();
      CascadeBuffer->SetArrayTexture(0, CascadeTextures, 0);
      CascadeBuffer->BeginCapture();
      DrawShadowShader->use();
      DrawShadowShader->setMat4("lightSpaceMatrix", directionalLights[i]->GetLightSpaceMatrix(0));

      DrawScene(renderObjs, currRenderObj, DrawShadowShader, &directionalLights[i]->GetFrustum(0));
      //CascadeVSMTextures->GenerateMipmap();

      // TRANSPARENT PASS
      if (directionalLights[i]->HaveTransparentShadow())
      {
        //directionalLights[i]->CaptureTransparentShadow();

        // enable color write, disable depth write
        glDepthMask(false);
        glDisable(GL_CULL_FACE);
        //glDisable(GL_DEPTH_TEST);

        // render opaque depth into color with quad
        /*DepthQuadShader->use();
        directionalLights[i]->GetFramebuffer()->GetTexture(0).texture->Bind(0);
        DepthQuadShader->setInt("srcTex", 0);
        quad->Draw();*/

        for (int j = 0; j < 1; j++)
        {
          glEnable(GL_BLEND);

          // set blend to min
          glBlendEquation(GL_MIN);
          glBlendFunc(GL_ONE, GL_ONE);

          DrawTransparentShadowShader->use();

          CascadeStochasticBuffer->BeginCapture();
#ifdef COLOR_STOCHASTIC_SHADOW
          static unsigned int vsmattachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
          glDrawBuffers(2, vsmattachments);
          glClear(GL_COLOR_BUFFER_BIT);
          glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.f, 1.f, 1.f, 0.f)[0]);
          glClearBufferfv(GL_COLOR, 1, &glm::vec4(1.f, 1.f, 1.f, 0.f)[0]);
#else
          glDrawBuffer(GL_COLOR_ATTACHMENT0);
          glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.f, 1.f, 1.f, 0.f)[0]);
#endif

          DrawTransparentShadowShader->setMat4("lightSpaceMatrix", directionalLights[i]->GetLightSpaceMatrix(j));
          //uniformNoise->Bind(0);
          //DrawTransparentShadowShader->setInt("uniformNoise", 0);
          for (int k = 0; k < currTransparentObjCount; k++)
          {
            if (FrustumCull(directionalLights[i]->GetFrustum(j), transparentObjs[k].boundingVolume) == true)
            {
              //transparentObjs[i].material->UpdateUniform(DrawTransparentShadowShader);
              DrawTransparentShadowShader->setFloat("property.alpha", transparentObjs[k].material->GetFloat("property.alpha"));
              DrawTransparentShadowShader->setFloat("property.transmission", transparentObjs[k].material->GetFloat("property.transmission"));
              DrawTransparentShadowShader->setVec3("property.diffuse", transparentObjs[k].material->GetVec3("property.albedo"));
              DrawTransparentShadowShader->setFloat("property.etaRatio", transparentObjs[k].material->GetFloat("property.etaRatio"));
              DrawTransparentShadowShader->setBool("property.haveDiffMap", true);
              DrawTransparentShadowShader->setInt("property.diffMap", 1);
              transparentObjs[k].material->GetSampler2DTex("property.diffMap")->Bind(1);
              transparentObjs[k].Draw(DrawTransparentShadowShader);
            }
          }

          glBlendEquation(GL_FUNC_ADD);
          glDisable(GL_BLEND);

#ifdef COLOR_STOCHASTIC_SHADOW
          // copy up sampled stochastic to down sampled vsm
          VSMCopyShader->use();
          VSMCopyShader->setInt("mean", 0);
          VSMCopyShader->setInt("variance", 1);
          CascadeStochasticBuffer->BindTexture(0, 0);
          CascadeStochasticBuffer->BindTexture(1, 1);

          int finalMeanLayerID = j * 2;
          int finalVarLayerID = j * 2 + 1;
          CascadeVSMFinalBuffer->SetArrayTexture(0, CascadeVSMTextures, finalMeanLayerID);
          CascadeVSMFinalBuffer->SetArrayTexture(1, CascadeVSMTextures, finalVarLayerID);
          CascadeVSMFinalBuffer->BeginCapture();
          glDrawBuffers(2, vsmattachments);
          
          quad->Draw();

          // filtering vsm
          GaussianFilterShader->use();
          GaussianFilterShader->setInt("texArray", 0);
          CascadeVSMTextures->Bind(0);

          float blurPerCascade = VSMBlurAmount / (float)CascadeVSMTextures->GetWidth() / (j + 1);

          // horizontal blur
          CascadeVSMBlurBuffer->BeginCapture();
          glDrawBuffers(2, vsmattachments);
          float meanDepth = CascadeVSMTextures->GetSampleDepth(finalMeanLayerID);
          float varDepth = CascadeVSMTextures->GetSampleDepth(finalVarLayerID);
          GaussianFilterShader->setVec2("meanVarDepth", glm::vec2(meanDepth, varDepth));
          GaussianFilterShader->setVec2("blurScale", glm::vec2(blurPerCascade, 0.f));
          quad->Draw();

          // variance blur filter
          CascadeVSMFinalBuffer->BeginCapture();
          glDrawBuffers(2, vsmattachments);
          meanDepth = CascadeVSMTextures->GetSampleDepth(cascadeTransparentPartition * 2);
          varDepth = CascadeVSMTextures->GetSampleDepth(cascadeTransparentPartition * 2 + 1);
          GaussianFilterShader->setVec2("meanVarDepth", glm::vec2(meanDepth, varDepth));
          GaussianFilterShader->setVec2("blurScale", glm::vec2(0.f, blurPerCascade));
          quad->Draw();
#else
          VSMCopyShader->use();
          VSMCopyShader->setInt("src", 0);
          CascadeStochasticBuffer->BindTexture(0, 0);

          CascadeVSMFinalBuffer->SetArrayTexture(0, CascadeVSMTextures, j);
          CascadeVSMFinalBuffer->BeginCapture();
          glDrawBuffer(GL_COLOR_ATTACHMENT0);
          quad->Draw();

          GaussianFilterShader->use();
          GaussianFilterShader->setInt("texArray", 0);
          CascadeVSMTextures->Bind(0);

          // horizontal blur
          CascadeVSMBlurBuffer->BeginCapture();
          glDrawBuffer(GL_COLOR_ATTACHMENT0);
          float depth = CascadeVSMTextures->GetSampleDepth(j);
          GaussianFilterShader->setFloat("sampleDepth", depth);
          GaussianFilterShader->setVec2("blurScale", glm::vec2(blurPerCascade, 0.f));
          quad->Draw();

          // variance blur filter
          CascadeVSMFinalBuffer->BeginCapture();
          glDrawBuffer(GL_COLOR_ATTACHMENT0);
          depth = CascadeVSMTextures->GetSampleDepth(cascadeTransparentPartition);
          GaussianFilterShader->setFloat("sampleDepth", depth);
          GaussianFilterShader->setVec2("blurScale", glm::vec2(0.f, blurPerCascade));
          quad->Draw();
#endif

          // downsample then do gaussian blur
          //directionalLights[i]->FilterVSMShadow(quad, VSMCopyShader, GaussianFilterShader);
        }

        CascadeVSMTextures->GenerateMipmap();

        glEnable(GL_CULL_FACE);
        glDepthMask(true);
        //glEnable(GL_DEPTH_TEST);
      }

      //ShadowAtlas->GetFrambuffer()->BeginCapture();
      //Framebuffer* dirFBO = directionalLights[i]->GetFramebuffer();
      //glBindFramebuffer(GL_READ_FRAMEBUFFER, dirFBO->GetFBO());
      //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ShadowAtlas->GetFrambuffer()->GetFBO());
      //glBlitFramebuffer(0, 0, dirFBO->GetTexture(1).texture->GetWidth(), dirFBO->GetTexture(1).texture->GetHeight(), 0, 0, ShadowAtlas->GetTexture()->GetWidth(), ShadowAtlas->GetTexture()->GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
      //ShadowAtlas->WriteToTexture(dirFBO->GetTexture(1).texture->GetID(), castShadowNum);

      castShadowNum++;
    }
    dirLightNum++;
  }
  offset = 0;
  UBlights->setInt(dirLightNum, offset);
  //UBlights->setInt(ShadowAtlas->GetGridPerRow(), offset);
}

void Renderer::UpdateCascadeCenter(int cascadeID, float& outNearDist, float& outFarDist, glm::vec3& outCenter)
{
  outNearDist = (cascadeID == 0) ? (camera->GetNear() / camera->GetFar()) : cascadeDistances[cascadeID - 1];
  outFarDist = cascadeDistances[cascadeID];
  float centerDist = outNearDist + (outFarDist - outNearDist) / 2.f;
  centerDist *= camera->GetFar();

  outCenter = camera->GetPosition() + camera->GetTarget() * centerDist;
}

void Renderer::DrawSkybox()
{
  Profiler::Begin("Skybox");

  GLint OldCullFaceMode;
  glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
  GLint OldDepthFuncMode;
  glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

  glCullFace(GL_FRONT);
  glDepthFunc(GL_LEQUAL);

  skyBox->GetMaterial()->GetShader()->use();
  skyBox->Draw();

  glCullFace(OldCullFaceMode);
  glDepthFunc(OldDepthFuncMode);

  Profiler::End();
}

void Renderer::OITPass()
{
  Profiler::Begin("Transparent pass");

  renderBuffer->CopyDepthBuffer(OTIBuffer, 0, 0);
  renderBuffer->CopyDepthBuffer(OpaqueBuffer, 0, 0);

  // copy opaque pass
  glDepthMask(false);
  OpaqueBuffer->BeginCapture();
  TextureQuadDepthShader->use();
  TextureQuadDepthShader->setInt("srcTex", 0);
  renderBuffer->BindTexture(0, 0);
  quad->Draw();
  OpaqueBuffer->EndCapture();

  // draw transparent objs
  OTIBuffer->BeginCapture(false);

  glClear(GL_COLOR_BUFFER_BIT);
  glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.f)[0]);
  glClearBufferfv(GL_COLOR, 1, &glm::vec4(1.f, 1.f, 1.f, 0.f)[0]);
  glClearBufferfv(GL_COLOR, 2, &glm::vec4(0.f)[0]);

  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glBlendFunci(0, GL_ONE, GL_ONE);
  glBlendFuncSeparatei(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ONE); // rgb: multiply, a: additive
  glBlendFunci(2, GL_ONE, GL_ONE);

  ClusterPhongAlphaShader->use();
  //skyBox->GetCubeMap()->Bind(3);
  //ClusterPhongAlphaShader->setInt("skybox", 3);
  //ClusterPhongAlphaShader->setInt("irradianceMap", 3);
  ClusterPhongAlphaShader->setInt("prefilterMap", 4);
  ClusterPhongAlphaShader->setInt("brdfLUT", 5);
  ClusterPhongAlphaShader->setSH("sh[0]", SphericalHarmonics);
  //ClusterPhongAlphaShader->setInt("depthTex", 6);
  //IrradianceMap->Bind(3);
  PrefilterMap->Bind(4);
  BRDF->Bind(5);
  //OpaqueBuffer->BindTexture(1,6);

  Shader* prevShader = nullptr;
  for (int i = 0; i < currTransparentObjCount; i++)
  {
    BoundingVolume& bd = transparentObjs[i].boundingVolume;
    if (FrustumCull(camera->GetFrustrum(), bd) == true)
    {
      Shader* currShader = transparentObjs[i].material->GetShader();
      if (prevShader != currShader)
      {
        SupplyShaderUniforms(currShader);
      }

      // find out background info for refraction

      // Estimate of distance from object to background to
      // be constant (we could read back depth buffer, but
      // that won't produce frame coherence)
      //const float zDistanceToHit = glm::max(2.0f, bd.radius);
      //const float backZ = cameraFrame.pointToObjectSpace(bd.center).z - zDistanceToHit;
      float backPlaneZ = glm::length(camera->GetPosition() - bd.center);//glm::min(-0.5f, backZ);
      //ClusterPhongAlphaShader->setFloat("backgroundZ", backPlaneZ);

      // Find out how big the back plane is in meters
      Frustum camFrust = camera->GetFrustrum();
      camFrust.UpdateProjection(camFrust.angle, camFrust.ratio, camFrust.nearD, backPlaneZ);
      camFrust.UpdateLookAt(camera->GetPosition(), glm::normalize(camera->GetTarget()), camera->GetUp());

      // Since we use the lengths only, do not bother taking to world space
      glm::vec2 backSizeMeters(glm::length(camFrust.ftr - camFrust.ftl), glm::length(camFrust.ftr - camFrust.fbr));
      ClusterPhongAlphaShader->setVec2("bgSizeMeter", backSizeMeters);

      transparentObjs[i].UpdateMaterialUniforms();
      transparentObjs[i].Draw(currShader);

      prevShader = currShader;
    }
  }

  OTIBuffer->EndCapture();

  // copy render buffer's color to tmp buffer
 /* glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_ONE, GL_ZERO);
  TextureQuadShader->use();
  TextureQuadShader->setInt("srcTex", 0);
  OTIBuffer->GetTexture(2).texture->Bind(0);
  quad->Draw();
  glEnable(GL_DEPTH_TEST);*/

  // 2D Compositing Pass
  renderBuffer->BeginCapture(false);
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  OITCombineShader->use();
  OITCombineShader->setInt("ATex", 0);
  OITCombineShader->setInt("BDTex", 1);
  OITCombineShader->setInt("DeltaTex", 2);
  OITCombineShader->setInt("BGTex", 3);
  OTIBuffer->BindTexture(0, 0);
  OTIBuffer->BindTexture(1, 1);
  OTIBuffer->BindTexture(2, 2);
  OpaqueBuffer->BindTexture(0, 3);
  quad->Draw();
  
  glDepthMask(true);

  Profiler::End();
}

void Renderer::DeferredShading()
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Profiler::Begin("Clustered Deferred");

  // update lights and global uniforms
  Profiler::Begin("Prepare lights");
  UpdateGlobalUniforms();
  UpdateLights();
  Profiler::End();

  // build clusters, only recalculate if camera's frustum (fov, near, far, ) changed
  Profiler::Begin("Calculate clusters");
  CalculateClusters();
  Profiler::End();

  // light culling
  Profiler::Begin("Light culling");
  ClusterLightCull();
  Profiler::End();

  // geometry pass
  Profiler::Begin("Geometry pass");
  gBuffer->BeginCapture();
  DrawSceneNoLight(renderObjs, currRenderObj, camera->GetFrustrum());
  gBuffer->EndCapture();
  Profiler::End();

  // SSAO
  Profiler::Begin("SSAO");
  SSAOBuffer->BeginCapture();
  SSAOShader->use();
  SSAOShader->setFloat("radius", ssaoRadius);
  SSAOShader->setFloat("bias", ssaoBias);
  SSAOShader->setFloat("power", ssaoPower);
  SSAOShader->setVec3Array("samples[0]", ssaoKernel);
  SSAOShader->setInt("gPos", 0);
  SSAOShader->setInt("gNorm", 1);
  SSAOShader->setInt("noise", 2);
  gBuffer->BindTexture(0, 0);
  gBuffer->BindTexture(1, 1);
  ssaoNoise->Bind(2);
  quad->Draw();

  // SSAO Blur
  SSAOBlur->BeginCapture();
  SSAOBlurShader->use();
  SSAOBlurShader->setInt("ssao", 0);
  SSAOBuffer->BindTexture(0, 0);
  quad->Draw();
  Profiler::End();

  // light pass
  Profiler::Begin("Light pass");
  renderBuffer->BeginCapture();
  glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPolygonMode(GL_FRONT_AND_BACK, ShaderConfig::PolygonMode::Fill);

  lightPassShader->use();
  gBuffer->BindAllTextures(lightPassShader, 0);
  
  lightPassShader->setInt("cascadeCount", 1);
  lightPassShader->setInt("cascadeVSMCount", 1);
  //lightPassShader->setFloat("cascadeEdgeBlend", cascadeBlendWidth);
  //lightPassShader->setFloatArray("cascadeDepths[0]", cascadeDistances);
  lightPassShader->setMat4Array("cascadeMatrices[0]", directionalLights[0]->GetLightSpaceMatrixArray());

  //pointLights[0]->GetFramebuffer()->GetTexture(0).texture->Bind(6);
  lightPassShader->setInt("shadowOpaque", 5);
  lightPassShader->setInt("shadowTransparent", 6);
  //lightPassShader->setInt("pointShadow", 6);
  lightPassShader->setInt("ssao", 8);
  //lightPassShader->setInt("irradianceMap", 9);
  lightPassShader->setInt("prefilterMap", 10);
  lightPassShader->setInt("brdfLUT", 11);
  lightPassShader->setSH("sh[0]", SphericalHarmonics);

  CascadeTextures->Bind(5);
  CascadeVSMTextures->Bind(6);
  SSAOBlur->BindTexture(0, 8);
  //IrradianceMap->Bind(9);
  PrefilterMap->Bind(10);
  BRDF->Bind(11);

  lightPassShader->setFloat("VSMMin", VSMMin);
  lightPassShader->setFloat("VSMBleedReduct", VSMLightBleedReduction);
  //lightPassShader->setVec2("MSMBleedReductDarken", MSMBleedReductDarken);
 
  quad->Draw();

  glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
  Profiler::End();

  // copy depth info
  gBuffer->CopyDepthBuffer(renderBuffer, 0, 0);

  // skybox
  DrawSkybox();

  // forward transparency pass
  OITPass();

  // post process
  PostProcess();

  Profiler::End();
}

void Renderer::ClusteredForward()
{
  Profiler::Begin("Clustered forward");

  Profiler::Begin("Prepare lights");

  UpdateGlobalUniforms();
  UpdateLights();

  Profiler::End();
  Profiler::Begin("Calculate clusters");

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(true);

  // build clusters, only recalculate if camera's frustum (fov, near, far, ) changed
  CalculateClusters();

  Profiler::End();
  Profiler::Begin("Depth prepass");

  // depth prepass
  /*DepthmapShader->use();

  depthBuffer->BeginCapture();

  DrawMaterial = false;
  DrawScene(DepthmapShader);

  depthBuffer->EndCapture();*/

  Profiler::End();
  
  // determine active clusters using prepass depth (optimization)
  /*Profiler::Begin("Cluster filtering");
  FilterClusters();
  Profiler::End();*/

  // light culling
  Profiler::Begin("Light culling");
  ClusterLightCull();
  Profiler::End();

  // forward drawing
  Profiler::Begin("Forward pass");

  //depthBuffer->CopyDepthBuffer(renderBuffer, 0, 0);

  renderBuffer->BeginCapture();
  glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  DrawScene(renderObjs, currRenderObj, &camera->GetFrustrum());

  Profiler::End();

  // skybox
  DrawSkybox();

  renderBuffer->EndCapture();

  // transparent drawing
  OITPass();

  Profiler::End();
}

void Renderer::CalculateClusters()
{
  // only recalculate if frustum changed
  if (camera->IsFrustumChanged() == false) return;

  camera->ResetFrustumChanged();

  if (ComputeClustersOnGPU)
  {
    CalcClustersGPU();
  }
  else
  {
    CalcClustersCPU();
  }
}

void Renderer::CalcClustersCPU()
{

}

void Renderer::CalcClustersGPU()
{
  // Building the grid of AABB enclosing the view frustum clusters
  buildAABBGridCompShader->use();
  buildAABBGridCompShader->setFloat("zNear", camera->GetNear());
  buildAABBGridCompShader->setFloat("zFar", camera->GetFar());
  buildAABBGridCompShader->dispatch(ClusterX, ClusterY, ClusterZ);
}

void Renderer::FilterClusters()
{
  // get loost active clusters
  filterClusterCompShader->use();
  filterClusterCompShader->setFloat("zNear", camera->GetNear());
  filterClusterCompShader->setFloat("zFar", camera->GetFar());
  depthBuffer->BindTexture(0, 0);
  filterClusterCompShader->setInt("depthBuffer", 0);
  filterClusterCompShader->dispatch(depthBuffer->GetWidht(), depthBuffer->GetHeight(), 1);

  filterCompactCompShader->use();
  filterClusterCompShader->dispatch(ClusterX, ClusterY, ClusterZ);
  /*bool activeClusters[16 * 9 * 24] = { false };
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 7, numClusters, activeClusters);

  for (int i = 0; i < numClusters; i++)
  {
    std::cout << activeClusters[i] << ", ";
  }
  std::cout << std::endl;*/

  // compact it into compacted array

}

void Renderer::ClusterLightCull()
{
  cullLightsCompShader->use();
  cullLightsCompShader->setMat4("viewMatrix", camera->GetLookAtMatrix());
  cullLightsCompShader->dispatch(1, 1, 6);
}

void Renderer::AddRenderObj(EntityID entity, unsigned int vao, unsigned int indiceCount, PrimitiveMode mode, const Transform& transform, Material* mat)
{
  if (mat->GetBlendMode() == ShaderConfig::BLEND_Translucent)
  {
    AssignTransparentObj(entity, vao, indiceCount, mode, transform, mat, BoundingVolume(transform.GetPosition(), transform.GetScale().x * 0.5f), nullptr);
  }
  else
  {
    AssignRenderObj(entity, vao, indiceCount, mode, transform, mat, BoundingVolume(transform.GetPosition(), transform.GetScale().x * 0.5f), nullptr);
  }
}

void Renderer::AddRenderObj(EntityID entity, unsigned int vao, unsigned int indiceCount, PrimitiveMode mode, const Transform& transform, Material* mat, std::vector<glm::mat4>* boneArray)
{
  if (mat->GetBlendMode() == ShaderConfig::BLEND_Translucent)
  {
    AssignTransparentObj(entity, vao, indiceCount, mode, transform, mat, BoundingVolume(transform.GetPosition(), transform.GetScale().x * 0.5f), boneArray);
  }
  else
  {
    AssignRenderObj(entity, vao, indiceCount, mode, transform, mat, BoundingVolume(transform.GetPosition(), transform.GetScale().x * 0.5f), boneArray);
  }
}

void Renderer::AddRenderObj(EntityID entity, unsigned int vao, unsigned int indiceCount, PrimitiveMode mode, const Transform& transform, Material* mat, const BoundingVolume& bv)
{
  if (mat->GetBlendMode() == ShaderConfig::BLEND_Translucent)
  {
    AssignTransparentObj(entity, vao, indiceCount, mode, transform, mat, bv, nullptr);
  }
  else
  {
    AssignRenderObj(entity, vao, indiceCount, mode, transform, mat, bv, nullptr);
  }
}

void Renderer::AssignRenderObj(EntityID entity, unsigned int vao, unsigned int indiceCount, PrimitiveMode mode, const Transform& transform, Material* mat, const BoundingVolume& bv, std::vector<glm::mat4>* boneArray)
{
  ASSERT(currRenderObj < maxRenderObj, "[Renderer]: ERROR! Max render object limit reached!");

  renderObjs[currRenderObj].entity = entity;
  renderObjs[currRenderObj].VAO = vao;
  renderObjs[currRenderObj].IndiceCount = indiceCount;
  renderObjs[currRenderObj].primitiveMode = mode;
  renderObjs[currRenderObj].transform = transform;
  renderObjs[currRenderObj].material = mat;
  renderObjs[currRenderObj].boundingVolume = bv;
  renderObjs[currRenderObj].boneArray = boneArray;

  currRenderObj++;
}

void Renderer::AssignTransparentObj(EntityID entity, unsigned int vao, unsigned int indiceCount, PrimitiveMode mode, const Transform& transform, Material* mat, const BoundingVolume& bv, std::vector<glm::mat4>* boneArray)
{
  ASSERT(currTransparentObjCount < maxTransparentObj, "[Renderer]: ERROR! Max transparent object limit reached!");

  transparentObjs[currTransparentObjCount].entity = entity;
  transparentObjs[currTransparentObjCount].VAO = vao;
  transparentObjs[currTransparentObjCount].IndiceCount = indiceCount;
  transparentObjs[currTransparentObjCount].primitiveMode = mode;
  transparentObjs[currTransparentObjCount].transform = transform;
  transparentObjs[currTransparentObjCount].material = mat;
  transparentObjs[currTransparentObjCount].boundingVolume = bv;
  transparentObjs[currTransparentObjCount].boneArray = boneArray;

  currTransparentObjCount++;
}

void Renderer::Update()
{
  Profiler::Begin("Upload render objs");

  currRenderObj = 0;
  currTransparentObjCount = 0;

  meshSystem->Draw();
  skinnedMeshSystem->Draw();
  clothSystem->Draw();

  Profiler::End();
}

void Renderer::SupplyShaderUniforms(Shader* shader)
{
  shader->use();

  shader->setInt("cascadeCount", cascadePartition);
  shader->setInt("cascadeVSMCount", cascadeTransparentPartition);
  //shader->setFloat("cascadeEdgeBlend", cascadeBlendWidth);
  //shader->setFloatArray("cascadeDepths[0]", cascadeDistances);
  shader->setMat4Array("cascadeMatrices[0]", directionalLights[0]->GetLightSpaceMatrixArray());

  //directionalLights[0]->GetFramebuffer()->GetTexture(1).texture->Bind(0);
  //ShadowAtlas->BindTextures(0);
  CascadeTextures->Bind(0);
  CascadeVSMTextures->Bind(1);
  shader->setInt("shadowOpaque", 0);
  shader->setInt("shadowTransparent", 1);
  //pointLights[0]->GetFramebuffer()->GetTexture(0).texture->Bind(1);
  //shader->setInt("pointShadow", 1);
  shader->setFloat("VSMMin", VSMMin);
  shader->setFloat("VSMBleedReduct", VSMLightBleedReduction);
}

void Renderer::DrawScene(std::vector<RenderObject>& objs, unsigned int objCount, Shader *shader, const Frustum* frustum)
{
  shader->use();

  if (frustum)
  {
    for (unsigned int i = 0; i < objCount; i++)
    {
      if (FrustumCull(*frustum, objs[i].boundingVolume) == true)
      {
        objs[i].Draw(shader);
      }
    }
  }
  else
  {
    for (unsigned int i = 0; i < objCount; i++)
    {
      objs[i].Draw(shader);
    }
  }
}

void Renderer::DrawScene(std::vector<RenderObject>& objs, unsigned int objCount, const Frustum* frustum)
{
  Shader* prevShader = nullptr;
  if (frustum)
  {
    for (unsigned int i = 0; i < objCount; i++)
    {
      if (FrustumCull(*frustum, objs[i].boundingVolume) == true)
      {
        Shader* currShader = objs[i].material->GetShader();
        if (prevShader != currShader)
        {
          SupplyShaderUniforms(currShader);
        }
        objs[i].UpdateMaterialUniforms();
        objs[i].Draw(currShader);

        prevShader = currShader;
      }
    }
  }
  else
  {
    for (unsigned int i = 0; i < objCount; i++)
    {
      Shader* currShader = objs[i].material->GetShader();
      if (prevShader != currShader)
      {
        SupplyShaderUniforms(currShader);
      }
      objs[i].UpdateMaterialUniforms();
      objs[i].Draw(currShader);

      prevShader = currShader;
    }
  }
}

void Renderer::DrawSceneNoLight(std::vector<RenderObject>& objs, unsigned int objCount, const Frustum &frustum)
{
  Shader* prevShader = nullptr;
  for (unsigned int i = 0; i < objCount; i++)
  {
    if (FrustumCull(frustum, objs[i].boundingVolume) == true)
    {
      Shader* currShader = objs[i].material->GetShader();
      if (prevShader != currShader)
      {
        currShader->use();
        prevShader = currShader;
      }
      objs[i].UpdateMaterialUniforms();
      objs[i].Draw(currShader);
    }
  }
}

void Renderer::PostProcess()
{
  Profiler::Begin("Post process");

  glDisable(GL_DEPTH_TEST);

  /*SSRBuffer->BeginCapture();
  SSRShader->use();
  SSRShader->setFloat("maxDist", ssrMaxDist);
  SSRShader->setFloat("resolution", ssrResolution);
  SSRShader->setFloat("steps", ssrSteps);
  SSRShader->setFloat("thickness", ssrThickness);
  SSRShader->setInt("gPos", 0);
  SSRShader->setInt("gNorm", 1);
  SSRShader->setInt("opaqueTex", 2);
  gBuffer->GetTexture(0).texture->Bind(0);
  gBuffer->GetTexture(1).texture->Bind(1);
  renderBuffer->GetTexture(0).texture->Bind(2);
  quad->Draw();*/

  // bloom filter
  PingpongBuffer[1]->BeginCapture();
  BloomFilter->use();
  renderBuffer->BindTexture(0, 0);
  BloomFilter->setInt("source", 0);
  BloomFilter->setFloat("threshold", bloomThreshold);
  quad->Draw();

  // bloom blur

  // horizontal blur
  KawaseBlurDown->use();
  KawaseBlurDown->setInt("src", 0);
  KawaseBlurDown->setVec2("offset", glm::vec2((float)bloomOffset));
  KawaseBlurDown->setVec2("halfPixel", glm::vec2(0.5f / (float)PingpongBuffer[0]->GetWidht(), 0.5f / (float)PingpongBuffer[0]->GetHeight()));
  int downOutput = 0;
  for (int i = 0; i < bloomIterations; i++)
  {
    downOutput = i % 2;
    int input = downOutput ? 0 : 1;
    PingpongBuffer[downOutput]->BeginCapture();
    PingpongBuffer[input]->BindTexture(0, 0);
    quad->Draw();
  }

  // vertical blur filter
  KawaseBlurUp->use();
  KawaseBlurUp->setInt("src", 0);
  KawaseBlurUp->setVec2("offset", glm::vec2((float)bloomOffset));
  KawaseBlurUp->setVec2("halfPixel", glm::vec2(0.5f / (float)PingpongBuffer[0]->GetWidht(), 0.5f / (float)PingpongBuffer[0]->GetHeight()));
  int upOutput = 0;
  for (int i = 0; i < bloomIterations; i++)
  {
    int input = (downOutput + i) % 2;
    upOutput = input ? 0 : 1;
    PingpongBuffer[upOutput]->BeginCapture();
    PingpongBuffer[input]->BindTexture(0, 0);
    quad->Draw();
  }

  // copy renderBuffer to opaque
  //OpaqueBuffer->BeginCapture();
  //TextureQuadShader->use();
  //renderBuffer->GetTexture(0).texture->Bind(0);
  //TextureQuadShader->setInt("srcTex", 0);
  //quad->Draw();

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  renderBuffer->BeginCapture(false);
  BloomCombine->use();
  PingpongBuffer[upOutput]->BindTexture(0, 0);
  BloomCombine->setInt("bloom", 0);
  BloomCombine->setFloat("bloomInten", bloomIntensity);
  quad->Draw();

  glDisable(GL_BLEND);

  finalOutput->BeginCapture();
  ColorCorrectionShader->use();
  ColorCorrectionShader->setFloat("exposure", exposure);
  ColorCorrectionShader->setInt("source", 0);
  renderBuffer->BindTexture(0, 0);
  quad->Draw();

  glEnable(GL_DEPTH_TEST);

  Profiler::End();
}

void Renderer::DrawFinalToQuad()
{
  TextureQuadShader->use();
  finalOutput->BindTexture(0, 0);
  TextureQuadShader->setInt("srcTex", 0);
  quad->Draw();
}

void Renderer::DrawDebug(Shader* shader)
{
  /*const Frustum& frus = camera->GetFrustrum();
  DebugLine(frus.fbl, frus.fbr, 1.f, glm::vec3(1.f, 0.f, 1.f));
  DebugLine(frus.ftl, frus.ftr, 1.f, glm::vec3(1.f, 0.f, 1.f));
  DebugLine(frus.ftl, frus.fbl, 1.f, glm::vec3(1.f, 0.f, 1.f));
  DebugLine(frus.ftr, frus.fbr, 1.f, glm::vec3(1.f, 0.f, 1.f));

  DebugLine(frus.nbl, frus.nbr, 1.f, glm::vec3(1.f, 0.f, 1.f));
  DebugLine(frus.ntl, frus.ntr, 1.f, glm::vec3(1.f, 0.f, 1.f));
  DebugLine(frus.ntl, frus.nbl, 1.f, glm::vec3(1.f, 0.f, 1.f));
  DebugLine(frus.ntr, frus.nbr, 1.f, glm::vec3(1.f, 0.f, 1.f));

  DebugLine(frus.nbl, frus.fbl, 1.f, glm::vec3(1.f, 0.f, 1.f));
  DebugLine(frus.nbr, frus.fbr, 1.f, glm::vec3(1.f, 0.f, 1.f));
  DebugLine(frus.ntl, frus.ftl, 1.f, glm::vec3(1.f, 0.f, 1.f));
  DebugLine(frus.ntr, frus.ftr, 1.f, glm::vec3(1.f, 0.f, 1.f));*/

  if (debugCascade)
  {
    for (int i = 0; i < cascadePartition; i++)
    {
      DebugLineCube(Transform(cascadeSpheres[i].center, glm::vec3(0.f, 1.f, 0.f), glm::vec3(3.f)));
    }
  }

  if (showBB)
  {
    for (unsigned int i = 0; i < currRenderObj; i++)
    {
      renderObjs[i].boundingVolume.Draw();
    }
    
  }
  if (showLightVolume)
  {
    for (PointLight* light : pointLights)
    {
      Renderer::DebugLineSphere(Transform(light->GetPosition(), glm::vec3(0.f, 1.f, 0.f), glm::vec3(light->GetRadius() * 2.f)), light->GetDiffuseColor());
    }
    for (DirectionalLight* light : directionalLights)
    {
      const Frustum& frus = light->GetFrustum(0);
      DebugLine(frus.fbl, frus.fbr, 1.f, glm::vec3(1.f, 0.f, 1.f));
      DebugLine(frus.ftl, frus.ftr, 1.f, glm::vec3(1.f, 0.f, 1.f));
      DebugLine(frus.ftl, frus.fbl, 1.f, glm::vec3(1.f, 0.f, 1.f));
      DebugLine(frus.ftr, frus.fbr, 1.f, glm::vec3(1.f, 0.f, 1.f));

      DebugLine(frus.nbl, frus.nbr, 1.f, glm::vec3(1.f, 0.f, 1.f));
      DebugLine(frus.ntl, frus.ntr, 1.f, glm::vec3(1.f, 0.f, 1.f));
      DebugLine(frus.ntl, frus.nbl, 1.f, glm::vec3(1.f, 0.f, 1.f));
      DebugLine(frus.ntr, frus.nbr, 1.f, glm::vec3(1.f, 0.f, 1.f));

      DebugLine(frus.nbl, frus.fbl, 1.f, glm::vec3(1.f, 0.f, 1.f));
      DebugLine(frus.nbr, frus.fbr, 1.f, glm::vec3(1.f, 0.f, 1.f));
      DebugLine(frus.ntl, frus.ftl, 1.f, glm::vec3(1.f, 0.f, 1.f));
      DebugLine(frus.ntr, frus.ftr, 1.f, glm::vec3(1.f, 0.f, 1.f));
    }
  }

  //for (unsigned int i = 0; i < pointLights.size(); i++)
  //{
  //  if (showLightIcon)
  //  {
  //    DebugSphere(Transform(pointLights[i]->GetPosition(), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.15f)), pointLights[i]->GetDiffuseColor());
  //  }
  //  if (showLightVolume)
  //  {
  //    Renderer::DebugLineCircleSphere(Transform(pointLights[i]->GetPosition(), glm::vec3(0.f, 1.f, 0.f), glm::vec3(pointLights[i]->GetRadius())), pointLights[i]->GetDiffuseColor());
  //  }
  //}

  //for (unsigned int i = 0; i < spotLights.size(); i++)
  //{
  //  if (showLightIcon)
  //  {
  //    DebugSphere(Transform(spotLights[i]->GetPosition(), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.15f)), spotLights[i]->GetDiffuseColor());
  //  }
  //  if (showLightVolume)
  //  {
  //    
  //    //Renderer::DebugLineSphere(Transform(pointLights[i]->GetPosition(), glm::vec3(0.f, 1.f, 0.f), glm::vec3(pointLights[i]->GetRadius())), pointLights[i]->GetDiffuseColor());
  //  }
  //}

  shader->use();

  /*for (BSpline* spline : debugSplines)
  {
    spline->Draw(shader);
  }
  debugSplines.clear();

  if (showGrid)
  {
    for (unsigned i = 0; i < 16; i++)
    {
      worldGrid[i].UpdateUniform(shader);
      debugMeshes[worldGrid[i].Type()]->Draw();
    }
  }*/

  for (unsigned i = 0; i < debugObjSize; i++)
  {
    if (debugObjects[i].Type() == DOT_Line)
    {
      debugObjects[i].UpdateMesh(debugMeshes[debugObjects[i].Type()]);
    }

    debugObjects[i].UpdateUniform(shader);
    debugMeshes[debugObjects[i].Type()]->Draw();
  }
  debugObjSize = 0;

  ShaderConfig::PolygonMode originalPM = Renderer::GetPolygonMode();
  Renderer::SetPolygonMode(ShaderConfig::Line);
  glDisable(GL_CULL_FACE);

  for (DebugModel& dm : debugModels)
  {
    dm.Draw(shader);
  }
  debugModels.clear();

  glEnable(GL_CULL_FACE);
  Renderer::SetPolygonMode(originalPM);

  finalOutput->EndCapture();

#ifdef MSAA_ON
  // now resolve multisampled buffer(s) into intermediate FBO
  glBindFramebuffer(GL_READ_FRAMEBUFFER, renderBuffer->GetFBO());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, finalOutput->GetFBO());
  glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  // now scene is stored as 2D texture image, so use that image for post-processing
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

void Renderer::SetCamera(Camera* _camera)
{
  camera = _camera;
  //FarPlane.x = camera->GetNear();
  //FarPlane.y = camera->GetFar();
}

void Renderer::AddPointLight(PointLight* light)
{
  if (pointLights.size() >= maxLights) return;

  pointLights.push_back(light);
}

void Renderer::AddSpotLight(SpotLight * light)
{
  if (spotLights.size() >= maxLights) return;

  spotLights.push_back(light);
}

void Renderer::AddDirectionalLight(DirectionalLight * light)
{
  if (directionalLights.size() >= maxLights) return;

  directionalLights.push_back(light);
}

PointLight* Renderer::GetPointLight(unsigned id)
{
  if (id < pointLights.size()) return pointLights[id];

  return nullptr;
}

SpotLight* Renderer::GetSpotLight(unsigned id)
{
  if (id < spotLights.size()) return spotLights[id];

  return nullptr;
}

DirectionalLight* Renderer::GetDirectionalLight(unsigned id)
{
  if (id < directionalLights.size()) return directionalLights[id];

  return nullptr;
}

void Renderer::RemovePointLight(unsigned id)
{
  if (id >= pointLights.size()) return;

  if (pointLights[id] == EditLight)
    EditLight = nullptr;

  delete pointLights[id];
  //lights[id].enabled = false;
  pointLights.erase(pointLights.begin() + id);
}

void Renderer::RemoveSpotLight(unsigned id)
{
  if (id >= spotLights.size()) return;

  if (spotLights[id] == EditLight)
    EditLight = nullptr;

  delete spotLights[id];
  spotLights.erase(spotLights.begin() + id);
}

void Renderer::RemoveDirectionalLight(unsigned id)
{
  if (id >= directionalLights.size()) return;

  if (directionalLights[id] == EditLight)
    EditLight = nullptr;

  delete directionalLights[id];
  directionalLights.erase(directionalLights.begin() + id);
}

void Renderer::SetSphericalHarmonics(float* image, int width, int height)
{
  SphericalHarmonics[0] = glm::vec3(0);
  SphericalHarmonics[1] = glm::vec3(0);
  SphericalHarmonics[2] = glm::vec3(0);
  SphericalHarmonics[3] = glm::vec3(0);
  SphericalHarmonics[4] = glm::vec3(0);
  SphericalHarmonics[5] = glm::vec3(0);
  SphericalHarmonics[6] = glm::vec3(0);
  SphericalHarmonics[7] = glm::vec3(0);
  SphericalHarmonics[8] = glm::vec3(0);
  SH::SphericalHarmonics(image, width, height, SphericalHarmonics);
}

void Renderer::LoadDebugMeshes()
{
  debugMeshes[DOT_Line] = ResourceManager::Get<Model>("debug line");
  debugMeshes[DOT_LineSquare] = ResourceManager::Get<Model>("debug line square");
  debugMeshes[DOT_LineCircle] = ResourceManager::Get<Model>("debug line circle");
  debugMeshes[DOT_LineCube] = ResourceManager::Get<Model>("debug line cube");
  debugMeshes[DOT_LineSphere] = ResourceManager::Get<Model>("debug line sphere");
  debugMeshes[DOT_Sphere] = ResourceManager::Get<Model>("debug sphere");
  debugMeshes[DOT_LineTriangle] = ResourceManager::Get<Model>("debug line triangle");
  debugMeshes[DOT_CircleSphere] = ResourceManager::Get<Model>("debug line circle sphere");
}

void Renderer::DebugLine(const glm::vec3& start, const glm::vec3& end, float width, const glm::vec3& color)
{
  /*glm::vec3 meshDir = glm::vec3(1.f, 0.f, 0.f);
  glm::vec3 dir = end - start;
  glm::vec3 scale(dir.length(), width, 1.f);
  glm::vec3 rotateAxis(glm::cross(meshDir, dir));
  float angle = glm::degrees(glm::acos(glm::dot(meshDir, dir) / dir.length()));
  Transform transform(start, rotateAxis, scale, angle);*/
  std::vector<glm::vec3> pos;
  pos.push_back(start);
  pos.push_back(end);

  debugObjects[debugObjSize].Init(DOT_Line, Transform(), color);
  debugObjects[debugObjSize].InitPoints(pos);
  debugObjSize++;
}

void Renderer::DebugLineSquare(const Transform & transform, const glm::vec3 & color)
{
  debugObjects[debugObjSize].Init(DOT_LineSquare, transform, color);
  debugObjSize++;
}

void Renderer::DebugLineCircle(const Transform & transform, const glm::vec3 & color)
{
  if (debugObjSize >= 10000)
  {
    return;
  }
  debugObjects[debugObjSize].Init(DOT_LineCircle, transform, color);
  debugObjSize++;
}

void Renderer::DebugLineCube(const Transform & transform, const glm::vec3 & color)
{
  if (debugObjSize >= 10000)
  {
    return;
  }
  debugObjects[debugObjSize].Init(DOT_LineCube, transform, color);
  debugObjSize++;
}

void Renderer::DebugLineSphere(const Transform & transform, const glm::vec3 & color)
{
  if (debugObjSize >= 10000)
  {
    return;
  }
  debugObjects[debugObjSize].Init(DOT_LineSphere, transform, color);
  debugObjSize++;
}

void Renderer::DebugSphere(const Transform & transform, const glm::vec3 & color)
{
  if (debugObjSize >= 10000)
  {
    return;
  }
  debugObjects[debugObjSize].Init(DOT_Sphere, transform, color);
  debugObjSize++;
}

void Renderer::DebugLineTriangle(const Transform & transform, const std::vector<glm::vec3> points, const glm::vec3 & color)
{
  if (debugObjSize >= 10000)
  {
    return;
  }
  debugObjects[debugObjSize].Init(DOT_LineTriangle, transform, color);
  debugObjects[debugObjSize].InitPoints(points);
  debugObjSize++;
}

void Renderer::DebugLineCircleSphere(const Transform& transform, const glm::vec3& color)
{
  if (debugObjSize >= 10000)
  {
    return;
  }
  debugObjects[debugObjSize].Init(DOT_CircleSphere, transform, color);
  debugObjSize++;
}

void Renderer::AddDebugModel(Model * model, const glm::vec3 & color)
{
  debugModels.push_back(DebugModel(model, color));
}

void Renderer::DebugBSpline(BSpline* spline)
{
  debugSplines.push_back(spline);
}

void Renderer::DrawDebugMenu(Rect<unsigned int> window)
{
  ImGui::BeginChild("Renderer Frame", ImVec2(window.ScaleX() - 20, window.ScaleY() - 20), true);
  ImGui::Text("Renderer Menu");

  ImGui::ColorEdit3("Clear Color", &clearColor[0]);

  static const char* shadingNames[] = { "Forward","Deferred","ForwardPlus" };
  static int shadingtechnique = shadingTechnique;
  if (ImGui::ListBox("Shading Technique", &shadingtechnique, shadingNames, ST_COUNT))
  {
    shadingTechnique = static_cast<ShadingTechnique>(shadingtechnique);
  }

  static bool wireframe = false;
  if (ImGui::Checkbox("Wireframe", &wireframe))
  {
    if (wireframe)
      Renderer::SetPolygonMode(ShaderConfig::Line);
    else
      Renderer::SetPolygonMode(ShaderConfig::Fill);
  }
  ImGui::Checkbox("Show Grid", &showGrid);
  ImGui::Checkbox("Show BB", &showBB);
  ImGui::Checkbox("Show Light Volumes", &showLightVolume);
  ImGui::Checkbox("Frustrum Culling", &FrustrumCulling);

  ImGui::ColorEdit3("Global Ambient Color", &GlobalAmbient[0]);
  ImGui::ColorEdit3("Fog Color", &FogColor[0]);
  ImGui::InputFloat("Fog Near Plane", &FarPlane[0]);
  ImGui::InputFloat("Fog Far Plane", &FarPlane[1]);

  if (ImGui::CollapsingHeader("Render Object List"))
  {
    ImGui::BeginChild("Obj List", ImVec2(300, 200), true);

    static bool addObjWindow;
    if (ImGui::Button("Add Object"))
    {
      addObjWindow = true;
    }
    if (addObjWindow)
    {
      ImGui::SetNextWindowSize(ImVec2(300, 300));
      ImGui::Begin("Create New Render Object", &addObjWindow);

      static char newRenderObjNameBuffer[512] = "unnamed object";
      static bool selectModel = false;
      static bool selectMaterial = false;
      static Model* modelToAdd = nullptr;
      static Material* materialToAdd = ResourceManager::Get<Material>("main");

      ImGui::InputText("Object Name", newRenderObjNameBuffer, 512);
      if (ImGui::Button("Select Model"))
      {
        selectModel = true;
      }
      if (selectModel)
      {
        ResourceManager::DrawResourceSelector<Model>(&modelToAdd, &selectModel);
      }
      ImGui::SameLine();
      if (ImGui::Button("Select Material"))
      {
        selectMaterial = true;
      }
      if (selectMaterial)
      {
        ResourceManager::DrawResourceSelector<Material>(&materialToAdd, &selectMaterial);
      }

      if (modelToAdd)
      {
        //ImGui::Image(ImTextureID(modelToAdd->GetPreviewTexture()), ImVec2(100.f, 100.f));
      }
      else
      {
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "WARNING: NO MESH SELECTED!");
      }

      ImGui::End();
    }

    ImGui::EndChild();
  }
  static bool addingLight = false;
  if (ImGui::CollapsingHeader("Light List"))
  {
    for (int i = 0; i < cascadePartition; i++)
    {
      std::string cascadename = "Cascade distance#"; cascadename += std::to_string(i);
      std::string cascadeUpdatename = "Cascade update#"; cascadeUpdatename += std::to_string(i);
      ImGui::InputFloat(cascadename.c_str(), &cascadeDistances[i]);
      ImGui::InputInt(cascadeUpdatename.c_str(), &cascadeUpdateRates[i]);
    }
    ImGui::InputFloat("Cascade edge blend width", &cascadeBlendWidth);
    ImGui::Checkbox("Debug cascade", &debugCascade);

    ImGui::InputFloat("Min lights distance", &minLightDistance);
    ImGui::InputFloat("Max lights distance", &maxLightDistance);

    ImGui::InputFloat("Min lights update rate", &minLightUpdateRate);
    ImGui::InputFloat("Max lights update rate", &maxLightUpdateRate);

    ImGui::BeginChild("L List", ImVec2(300, 200), true);

    if (ImGui::Button("Add Light"))
    {
      addingLight = true;
    }
    ImGui::SameLine();
    ImGui::Checkbox("Show Light Volume", &showLightVolume);
    ImGui::Checkbox("Show Light Icon", &showLightIcon);
    ImGui::Separator();
    for (unsigned int i = 0; i < directionalLights.size(); i++)
    {
      if (ImGui::Button(directionalLights[i]->GetName().c_str()))
      {
        Editor::SelectMenu("Right Editor", "Object Editor");
        Editor::SetObjEditor(DrawLightEditor);
        EditLight = directionalLights[i];
      }

      if (i != 0)
      {
        ImGui::SameLine();
        if (ImGui::Button(("Remove " + directionalLights[i]->GetName()).c_str()))
        {
          RemoveDirectionalLight(i);
        }
      }
    }

    for (unsigned int i = 0; i < pointLights.size(); i++)
    {
      if (ImGui::Button(pointLights[i]->GetName().c_str()))
      {
        Editor::SelectMenu("Right Editor", "Object Editor");
        Editor::SetObjEditor(DrawLightEditor);
        EditLight = pointLights[i];
      }
      ImGui::SameLine();
      if (ImGui::Button(("Remove " + pointLights[i]->GetName()).c_str()))
        RemovePointLight(i);
    }

    for (unsigned int i = 0; i < spotLights.size(); i++)
    {
      if (ImGui::Button(spotLights[i]->GetName().c_str()))
      {
        Editor::SelectMenu("Right Editor", "Object Editor");
        Editor::SetObjEditor(DrawLightEditor);
        EditLight = spotLights[i];
      }
      ImGui::SameLine();
      if (ImGui::Button(("Remove " + spotLights[i]->GetName()).c_str()))
        RemoveSpotLight(i);
    }

    ImGui::EndChild();
  }

  static glm::vec3 lightcolor;
  static glm::vec3 lightposition;
  static char nameBuffer[256];
  static int lighttype = LT_Point;
  static const char* lighttypes[LT_COUNT] = { "Directional", "Point", "Spotlight", "Skylight" };

  if (addingLight)
  {
    ImGui::SetNextWindowSize(ImVec2(300, 300));
    ImGui::Begin("New Light");

    ImGui::ListBox("light type", &lighttype, lighttypes, LT_COUNT);
    ImGui::InputText("light name", nameBuffer, 255);
    ImGui::ColorEdit3("light color", &lightcolor[0]);
    ImGui::InputFloat3("light position", &lightposition[0]);

    if (ImGui::Button("Create Light"))
    {
      switch (lighttype)
      {
      case(LT_Directional):
      {
        AddDirectionalLight(new DirectionalLight(nameBuffer, glm::vec3(0.5f, -0.5f, 0.f)));
        break;
      }
      case(LT_Point):
      {
        AddPointLight(new PointLight(nameBuffer, lightposition, 8.f, lightcolor, false));
        break;
      }
      case(LT_Spotlight):
      {
        AddSpotLight(new SpotLight(nameBuffer, lightposition, glm::vec3(0.0f, -1.0f, 0.f), 18.f, 25.f, 1.f, 0.14f, 0.07f, lightcolor, false));
        break;
      }
      }
      addingLight = false;
    }

    ImGui::End();
  }

  if (ImGui::CollapsingHeader("Depth prespass"))
  {
    depthBuffer->DrawDebugMenu();
  }

  if (ImGui::CollapsingHeader("Shadow Atlas"))
  {
    ShadowAtlas->DrawDebugMenu();

    /*static glm::vec4 shadowUV;
    ImGui::InputFloat4("Shadow UV", &shadowUV[0]);
    lightPassShader->use();
    lightPassShader->setVec4("shadowUV", shadowUV);*/
  }

  if (ImGui::CollapsingHeader("G-buffer"))
  {
    if (ImGui::Button("Reload Geomtry Shader"))
    {
      gBufferShader->Reload();
    }
    ImGui::Checkbox("Copy Depth Info", &copyDepthInfo);
    static const char* gBufferChannels[] = { "Light Pass", "Position", "Normal", "Ambient", "Diffuse", "Specular", "Shininess", "ShadowMap" };
    ImGui::ListBox("Display Channel", &currentGBufferChannel, gBufferChannels, 8);
    gBuffer->DrawDebugMenu();
  }

  if (ImGui::CollapsingHeader("Final output"))
  {
    finalOutput->DrawDebugMenu();
  }

  if (ImGui::CollapsingHeader("Textures Atlas"))
  {
    textureAtlas->DrawDebugMenu();
  }
  if (ImGui::CollapsingHeader("PBR"))
  {
    ImGui::Image(ImTextureID(BRDF->GetID()), ImVec2(80, 80));
    ImGui::InputFloat("Prefilter mip", &prefilterMip);
  }
  if (ImGui::CollapsingHeader("Editor Camera"))
  {
    camera->DrawDebugMenu();
  }
  if (ImGui::CollapsingHeader("Skybox"))
  {
    skyBox->DrawDebugMenu();
  }
  if (ImGui::CollapsingHeader("Opaque texture"))
  {
    OpaqueBuffer->DrawDebugMenu();
  }
  if (ImGui::CollapsingHeader("OIT Textures"))
  {
    OTIBuffer->DrawDebugMenu();
  }
  if (ImGui::CollapsingHeader("Pingpong buffers"))
  {
    PingpongBuffer[0]->DrawDebugMenu();
    PingpongBuffer[1]->DrawDebugMenu();
  }
  if (ImGui::CollapsingHeader("Stochastic VSM"))
  {
    ImGui::InputFloat("VSM Blur Amount", &VSMBlurAmount);
    ImGui::InputFloat("VSM Min", &VSMMin);
    ImGui::InputFloat("VSM Light Bleeding Reduction", &VSMLightBleedReduction);
    ImGui::InputFloat("MSM Light Bleeding Reduction", &MSMBleedReductDarken[0]);
    ImGui::InputFloat("MSM Darken", &MSMBleedReductDarken[1]);
  }
  if (ImGui::CollapsingHeader("SSR"))
  {
    ImGui::InputFloat("SSR Max Distance", &ssrMaxDist);
    ImGui::InputFloat("SSR Resolution", &ssrResolution);
    ImGui::InputFloat("SSR Steps", &ssrSteps);
    ImGui::InputFloat("SSR Thickness", &ssrThickness);
    SSRBuffer->DrawDebugMenu();
  }
  if (ImGui::CollapsingHeader("Post process"))
  {
    ImGui::InputFloat("Exposure", &exposure);
    ImGui::InputFloat("Bloom Threshold", &bloomThreshold);
    ImGui::InputFloat("Bloom Radius", &bloomRadius);
    ImGui::InputFloat("Bloom Intensity", &bloomIntensity);
    ImGui::InputInt("Bloom Offset", &bloomOffset);
    ImGui::InputInt("Bloom Iterations", &bloomIterations);
    ImGui::Separator();
    ImGui::InputFloat("SSAO radius", &ssaoRadius);
    ImGui::InputFloat("SSAO bias", &ssaoBias);
    ImGui::InputFloat("SSAO Blur", &ssaoBlur);
    ImGui::InputFloat("SSAO Power", &ssaoPower);
    ImGui::Image(ImTextureID(ssaoNoise->GetID()), ImVec2(80, 80));
    SSAOBuffer->DrawDebugMenu();
  }

  ImGui::EndChild();
}

void Renderer::DrawDisplayMenu(Rect<unsigned int> window)
{
  //ASSERT(Renderer::GetInstance(), "Renderer debug menu: null instance ptr.");

  const char* rendermodes[] = { "Shaded", "Wireframe", "Shaded Wireframe" };
  static const char* currentMode = rendermodes[0];

  //ImGui::SetNextWindowSizeConstraints(ImVec2(20, 40), ImVec2(20, 60));
  ImGui::PushItemWidth(140);
  if (ImGui::BeginCombo("##combo", currentMode))
  {
    for (int i = 0; i < 3; i++)
    {
      bool is_selected = (currentMode == rendermodes[i]); // You can store your selection however you want, outside or inside your objects
      if (ImGui::Selectable(rendermodes[i], is_selected))
        currentMode = rendermodes[i];
      if (is_selected)
        ImGui::SetItemDefaultFocus();
    }

    ImGui::EndCombo();
  }

  /*ImGui::SameLine();
  ImGui::Checkbox("Toggle Lighting", );
  ImGui::SameLine();
  ImGui::Checkbox("Toggle Audio", );*/
  ImGui::SameLine();
  if (ImGui::Button("Edit Camera"))
  {
    EditCam = camera;
    Editor::SetObjEditor(DrawCamEditor);
    //camera->DrawDebugMenu();
  }

  //ImGui::Checkbox("Show Debug Objects", );

  ImGui::Image(ImTextureID(finalOutput->GetTexture(0).texture->GetID()), ImVec2(finalOutput->GetWidht(), finalOutput->GetHeight()), ImVec2(0, 1), ImVec2(1, 0));
}

void Renderer::DrawRenderObjEditor(Rect<unsigned int> window)
{
  //if (EditObj) EditObj->DrawDebugMenu();
}

void Renderer::DrawLightEditor(Rect<unsigned int> window)
{
  if (EditLight) EditLight->DrawDebugMenu();
}

void Renderer::DrawCamEditor(Rect<unsigned int> window)
{
  if (EditCam) EditCam->DrawDebugMenu();
}

bool Renderer::FrustumCull(const Frustum& frustum, const BoundingVolume& bv)
{
  BoundingVolumeTypes type = bv.GetType();
  if (type == BoundingVolumeTypes::BVT_AABB)
  {
    return frustum.FastCullAABB(bv.max, bv.min);
  }
  else
  {
    return frustum.FastCullSphere(bv.center, bv.radius);
  }
  return false;
}