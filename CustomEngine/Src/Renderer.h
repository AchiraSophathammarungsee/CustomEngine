#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include "DebugObject.h"
#include <glad/glad.h>
#include "Rect.h"
#include "System.h"
#include "ShaderConfig.h"
#include "Light.h"
#include "RenderObject.h"
#include "Plane.h"
#include "Frustum.h"
#include "Skybox.h"
#include "GraphicAPI.h"
#include "SphericalHarmonics.h"

class DebugObject;
class Shader;
class Model;
class Light;
class PointLight;
class SpotLight;
class DirectionalLight;
class UniformBuffer;
class Camera;
class TextureAtlas;
class Framebuffer;
class MeshSystem;
class SkinnedMeshSystem;
class BSpline;
class ClothSystem;
class ComputeShader;
class Texture;
class TextureArray;

enum ShadingTechnique { ST_Forward, ST_Deferred, ST_ForwardPlus, ST_COUNT };
enum GbufferChannel { GBC_Position, GBC_Normal, GBC_Ambient, GBC_Diffuse, GBC_Specular, GBC_Shininess, GBC_Shadow, GBC_COUNT };

struct VolumeTileAABB
{
  glm::vec4 minPoint; // vec4 for memory alignment purpose
  glm::vec4 maxPoint;
};

struct ScreenToView
{
  glm::mat4 inverseProjectionMat;
  unsigned int tileSizes[4];
  unsigned int screenWidth;
  unsigned int screenHeight;
  float sliceScalingFactor;
  float sliceBiasFactor;
};

struct FrustrumSphere
{
  glm::vec3 center;
  float radius;
};

// Currently only used in the generation of SSBO's for light culling and rendering
// I think it potentially would be a good idea to just have one overall light struct for all light types
// and move all light related calculations to the gpu via compute or frag shaders. This should reduce the
// number of Api calls we're currently making and also unify the current lighting path that is split between 
// compute shaders and application based calculations for the matrices.
struct GPULight 
{
  glm::vec4 position;
  glm::vec4 color;
  unsigned int enabled;
  float intensity;
  float range;
  float haveShadow;
};

// clustered forward render pipeline:
//   depth prepass -> light setup -> opaque sort -> opaque pass -> sky pass -> SSAO -> Subsurface -> Screen reflection -> Combine -> translucent sort -> translucent pass -> Combine -> Postprocess
class Renderer
{
public:
  static void Init(std::shared_ptr<MeshSystem> meshSystem, std::shared_ptr<SkinnedMeshSystem> skinnedMeshSystem, unsigned width, unsigned height, unsigned maxlights, ShadingTechnique shadingTechnique = ST_Deferred);
  static void Update();
  static void Exit();

  static void Draw();
  // drawing scene with custom shader
  static void DrawScene(std::vector<RenderObject> &objs, unsigned int objCount, Shader* shader, const Frustum* frustum);
  // drawing scene with material on
  static void DrawScene(std::vector<RenderObject> &objs, unsigned int objCount, const Frustum* frustum);
  static void DrawSceneNoLight(std::vector<RenderObject> &objs, unsigned int objCount, const Frustum& frustum);
  static void DrawDebug(Shader* shader);

  static void DrawFinalToQuad();

  static void PostProcess();

  static void AddRenderObj(EntityID entity, unsigned int vao, unsigned int indiceCount, PrimitiveMode mode, const Transform &transform, Material *mat);
  static void AddRenderObj(EntityID entity, unsigned int vao, unsigned int indiceCount, PrimitiveMode mode, const Transform &transform, Material *mat, std::vector<glm::mat4> *boneArray);
  static void AddRenderObj(EntityID entity, unsigned int vao, unsigned int indiceCount, PrimitiveMode mode, const Transform &transform, Material *mat, const BoundingVolume &bv);

  static unsigned int Width() { return width; };
  static unsigned int Height() { return height; };
  static void SetViewRect(const Rect<unsigned int>& rect) { viewRect = rect; };
  static const Rect<unsigned int>& GetViewRect() { return viewRect; };

  static void SetClearColor(const glm::vec3& color) { clearColor = color; };
  static ShaderConfig::PolygonMode GetPolygonMode() { return polygonMode; };
  static void SetPolygonMode(ShaderConfig::PolygonMode mode) { polygonMode = mode; glPolygonMode(GL_FRONT_AND_BACK, polygonMode); };

  static void SetCamera(Camera* _camera);
  static bool FrustumCull(const Frustum &frustum, const BoundingVolume &bv);

  static void AddPointLight(PointLight* light);
  static void AddSpotLight(SpotLight* light);
  static void AddDirectionalLight(DirectionalLight* light);
  static PointLight* GetPointLight(unsigned id);
  static SpotLight* GetSpotLight(unsigned id);
  static DirectionalLight* GetDirectionalLight(unsigned id);
  static void RemovePointLight(unsigned id);
  static void RemoveSpotLight(unsigned id);
  static void RemoveDirectionalLight(unsigned id);
  static unsigned GetPointLightSize() { return pointLights.size(); };
  static unsigned GetSpotLightSize() { return spotLights.size(); };
  static unsigned GetDirectionalLightSize() { return directionalLights.size(); };

  static void SetSkyBoxMaterial(Material* mat) { skyBox->SetMaterial(mat); };

  static void LoadDebugMeshes();
  static void DebugLine(const glm::vec3& start, const glm::vec3& end, float width = 1.f, const glm::vec3& color = glm::vec3(1.f));
  static void DebugLineSquare(const Transform& transform, const glm::vec3& color = glm::vec3(1.f));
  static void DebugLineCircle(const Transform& transform, const glm::vec3& color = glm::vec3(1.f));
  static void DebugLineCube(const Transform& transform, const glm::vec3& color = glm::vec3(1.f));
  static void DebugLineSphere(const Transform& transform, const glm::vec3& color = glm::vec3(1.f));
  static void DebugLineCircleSphere(const Transform& transform, const glm::vec3& color = glm::vec3(1.f));
  static void DebugLineTriangle(const Transform& transform, const std::vector<glm::vec3> points, const glm::vec3& color = glm::vec3(1.f));
  static void DebugSphere(const Transform& transform, const glm::vec3& color = glm::vec3(1.f));
  static void AddDebugModel(Model* model, const glm::vec3& color = glm::vec3(1.f));

  static void DebugBSpline(BSpline *spline);

  static void DrawDebugMenu(Rect<unsigned int> window);
  static void DrawDisplayMenu(Rect<unsigned int> window);
  static void DrawRenderObjEditor(Rect<unsigned int> window);
  static void DrawLightEditor(Rect<unsigned int> window);
  static void DrawCamEditor(Rect<unsigned int> window);

  static void SetSphericalHarmonics(float *image, int width, int height);

  static void SetClothSystem(std::shared_ptr<ClothSystem> system) { clothSystem = system; };

private:
  static void DeferredShading();
  static void ClusteredForward();
  static void OITPass();

  static void SupplyShaderUniforms(Shader* shader);

  static void AssignRenderObj(EntityID entity, unsigned int vao, unsigned int indiceCount, PrimitiveMode mode, const Transform& transform, Material* mat, const BoundingVolume& bv, std::vector<glm::mat4>* boneArray);
  static void AssignTransparentObj(EntityID entity, unsigned int vao, unsigned int indiceCount, PrimitiveMode mode, const Transform& transform, Material* mat, const BoundingVolume& bv, std::vector<glm::mat4>* boneArray);

  static void GenShadowBufferPools();
  static void InitSSBOs();

  // can be done on CPU, not recommended (use computer shader for best performance)
  static bool ComputeClustersOnGPU;
  static void CalculateClusters();
  static void CalcClustersCPU();
  static void CalcClustersGPU();
  static void FilterClusters();
  static void ClusterLightCull();

  static void UpdateGlobalUniforms();
  static void UpdateLights();

  static void DrawSkybox();

  static void UpdateCascadeCenter(int cascadeID, float &outNearDist, float &outFarDist, glm::vec3 &outCenter);
  static void UpdateCascade();

  static std::shared_ptr<MeshSystem> meshSystem;
  static std::shared_ptr<SkinnedMeshSystem> skinnedMeshSystem;
  static std::shared_ptr<ClothSystem> clothSystem;

  static unsigned width, height;
  static Rect<unsigned int> viewRect;
  static bool FrustrumCulling;

  static ShaderConfig::PolygonMode polygonMode;
  static glm::vec3 clearColor;

  static ShadingTechnique shadingTechnique;
  static bool DrawMaterial;

  static unsigned int maxRenderObj;
  static unsigned int currRenderObj;
  static std::vector<RenderObject> renderObjs;

  static unsigned int maxTransparentObj;
  static unsigned int currTransparentObjCount;
  static std::vector<RenderObject> transparentObjs;

  static unsigned maxLights;
  static unsigned int prevPointLightCount;
  static float minLightDistance;
  static float maxLightDistance;
  static float minLightUpdateRate;
  static float maxLightUpdateRate;
  static std::vector<PointLight*> pointLights;
  static std::vector<SpotLight*> spotLights;
  static std::vector<DirectionalLight*> directionalLights;
  static bool showLightIcon;
  static bool showLightVolume;

  static std::vector<DebugObject> debugObjects;
  static std::vector<DebugModel> debugModels;
  static unsigned debugObjSize;
  static Model* debugMeshes[DOT_COUNT];

  static std::vector<BSpline*> debugSplines;

  static bool showBB;

  static Camera* camera;

  static UniformBuffer* UBmatrices;
  static UniformBuffer* UBlights;
  static Shader* TextureQuadShader;
  static Shader* TextureQuadDepthShader;
  static Shader* DefaultShader;
  static Shader* DepthmapShader;
  static Shader* DrawShadowShader;
  static Shader* DrawCubeShadowShader;
  static Shader* DrawTransparentShadowShader;
  static Shader* DepthQuadShader;
  static Shader* gBufferShader;
  static Shader* gBufferSkinnedShader;
  static Shader* gBufferPreviewShaders[GBC_COUNT];
  static Shader* lightPassShader;
  static Shader* ClusterPhongShader;
  static Shader* ClusterPhongAlphaShader;
  static Shader* OITCopyOpaqueShader;
  static Shader* OITCombineShader;
  static Shader* GaussianFilterShader;
  static Shader* Gaussian5x5Shader;
  static Shader* Gaussian9x9Shader;
  static Shader* ConvertStochasticVSM;
  static Shader* VSMCopyShader;
  static Shader* BloomFilter, *BloomCombine;
  static Shader* SSAOShader;
  static Shader* SSAOBlurShader;
  static Shader* KawaseBlurUp;
  static Shader* KawaseBlurDown;
  static Shader* MomentShadow;
  static Shader* MomentBlur;
  static Framebuffer* gBuffer;
  static Framebuffer* finalOutput;
  static Framebuffer* depthBuffer;
  static Framebuffer* renderBuffer;
  static Framebuffer* OTIBuffer;
  static Framebuffer* OpaqueBuffer;
  static Framebuffer* PingpongBuffer[2];
  static Framebuffer* SSAOBuffer, *SSAOBlur;
  static int currentGBufferChannel;
  static bool copyDepthInfo;
  static Model* quad;
  static Texture* uniformNoise;

  static TextureAtlas* textureAtlas;

  static glm::vec3 FogColor;
  static glm::vec3 GlobalAmbient;
  static glm::vec2 FarPlane;

  static DebugObject worldGrid[16];
  static bool showGrid;
  
  static Light* EditLight;
  static RenderObject* EditObj;
  static Camera* EditCam;

  static unsigned int ClusterX;
  static unsigned int ClusterY;
  static unsigned int ClusterZ;
  static unsigned int numClusters;
  static unsigned int sizeX, sizeY;

  static unsigned int AABBvolumeGridSSBO;
  static unsigned int screenToViewSSBO;
  static unsigned int activeClusterSSBO;
  static unsigned int globalCompactCountSSBO;
  static unsigned int compactClusterSSBO;
  static unsigned int lightSSBO;
  static unsigned int lightIndexListSSBO;
  static unsigned int lightGridSSBO;
  static unsigned int lightIndexGlobalCountSSBO;
  static unsigned int maxLightsPerTile;

  static ScreenToView screen2View;
  static ComputeShader* buildAABBGridCompShader;
  static ComputeShader* filterClusterCompShader;
  static ComputeShader* filterCompactCompShader;
  static ComputeShader* cullLightsCompShader;
  static Shader* cluserVisualShader;
  static struct GPULight* lights;

  static float VSMBlurAmount;
  static float VSMMin;
  static float VSMLightBleedReduction;
  static glm::vec2 MSMBleedReductDarken;

  static Skybox *skyBox;

  static int bloomDownsample;
  static float bloomThreshold;
  static float bloomRadius;
  static float bloomIntensity;
  static int bloomOffset;
  static int bloomIterations;

  static std::vector<glm::vec3> ssaoKernel;
  static Texture* ssaoNoise;
  static float ssaoRadius;
  static float ssaoBias;
  static float ssaoBlur;
  static float ssaoPower;

  static float time;
  static int temporalIndex;
  static int temporalPrevIndex;
  static int temporalMax;

  static bool debugCascade;
  static int cascadePartition;
  static int cascadeTransparentPartition;
  static int cascadeResolutions;
  static int stochasticUpsample;
  static int stochasticDownsample;
  static float cascadeBlendWidth;
  static std::vector<float> cascadeDistances;
  static std::vector<int> cascadeUpdateRates;
  static std::vector<int> cascadeUpdateTimers;
  static std::vector<FrustrumSphere> cascadeSpheres;
  static std::vector<unsigned int> ShadowResolutions;
  static std::vector<Framebuffer*> ShadowBufferPool;
  static std::vector<Framebuffer*> VSMBufferPool;
  static std::vector<Framebuffer*> VSMBlurBufferPool;
  static std::vector<Framebuffer*> VSMFinalBufferPool;
  static TextureAtlas* ShadowAtlas;
  static TextureAtlas* VSMShadowAtlas;
  static TextureArray* CascadeTextures;
  static Texture* CascadeStochasticTextures;
  static TextureArray* CascadeVSMTextures;
  static Framebuffer* CascadeBuffer;
  static Framebuffer* CascadeBlurBuffer;
  static Framebuffer* CascadeStochasticBuffer;
  static Framebuffer* CascadeVSMBlurBuffer;
  static Framebuffer* CascadeVSMFinalBuffer;

  static CubeMap* IrradianceMap;
  static CubeMap* PrefilterMap;
  static Texture* BRDF;
  static float prefilterMip;

  static float ssrMaxDist;
  static float ssrResolution;
  static float ssrSteps;
  static float ssrThickness;
  static Framebuffer* SSRBuffer;
  static Shader* SSRShader;

  static Shader* ColorCorrectionShader;
  static float exposure;

  static bool MSAA;

  static SH::SH_coeff SphericalHarmonics;
};

#endif