#include "Scene1.h"
#include "shader.hpp"
#include <glm/vec3.hpp>
#include <glm/detail/type_mat.hpp>
#include "ModelLoader.h"
#include "Model.h"
#include "Material.h"
#include "Camera.h"
#include "Light.h"
#include "Texture.h"
#include "Cubemap.h"
#include <algorithm>
#include "ModelGenerator.h"
#include "Framebuffer.h"
#include "UVWrap.h"
#include "ResourceManager.h"
#include "RenderObject.h"
#include "Renderer.h"
#include "Window.h"
#include "UniformBuffer.h"
#include "BoundingVolumeHierarchy.h"
#include "Octree.h"
#include "BSPTree.h"
#include "Editor.h"
#include "Transform.h"
#include "TestComp.h"
#include "TestSystem.h"
#include "Coordinator.h"
#include "MeshComp.h"
#include "Movement.h"
#include "Mass.h"
#include "Gravity.h"
#include "Physics.h"
#include "Collider.h"
#include "CollisionSolver.h"
#include "Quaternion.h"
#include "Vec.h"
#include "SkinnedMesh.h"
#include "SkinnedMeshComp.h"
#include "MeshSystem.h"
#include "SkinnedMeshSystem.h"
#include "SkeletalAnimation.h"
#include "SkeletalAnimator.h"
#include "BSpline.h"
#include "SplinePathSystem.h"
#include "IKSystem.h"
#include "IKAnimation.h"
#include "ClothComp.h"
#include "ClothSystem.h"
#include "ColliderSystem.h"
#include "ColliderManager.h"
#include "AudioSource.h"
#include "AudioSourceSystem.h"
#include "Profiler.h"
#include "MathUtil.h"
#include "ShaderParser.h"
#include "GraphicAPI.h"
#include "TextureLoader.h"
#include "FuzzyAI.h"
#include "FuzzyAISystem.h"

# define M_PI 3.14159265358979323846  /* pi */

std::shared_ptr<MeshSystem> meshSystem;
std::shared_ptr<SkinnedMeshSystem> skinnedMeshSystem;
std::shared_ptr<SkeletalAnimator> skeletalAnimatorSystem;
std::shared_ptr<SplinePathSystem> splinePathSystem;
std::shared_ptr<IKSystem> ikSystem;
std::shared_ptr<ClothSystem> clothSystem;
std::shared_ptr<Physics> physics;
std::shared_ptr<Gravity> gravity;
std::shared_ptr<CollisionSolver> collisionSolver;
std::shared_ptr<TestSystem> testsys;
std::shared_ptr<ColliderSphereSystem> sphereSys;
std::shared_ptr<ColliderBoxSystem> boxSys;
std::shared_ptr<AudioSourceSystem> audioSrcSys;
std::shared_ptr<FuzzyAISystem> fuzzyAISys;

BSpline* spline;

int lightCycle = 0;

Shader *mainShader;
Shader *normalLineShader;
Shader *depthMapShader;
Shader *gBufferShader;
Shader *gBufferSkinnedShader;
Shader *lightPassShader;
Shader *skyboxShader;

bool wireframe = false;

glm::mat4 projectionMat;
Camera *camera;

bool drawNormalLine = false;

bool drawOrbitLine = false;
float orbitRadius = 2.f;
int sphereNum = 8;
float orbitRotation = 0.f;
bool orbit = true;
bool lightRotateWithOrbit = true;

float DT = 0.f;
float lastFrame = 0.f;

float dLightAngle = 0.f;
bool dLightRotate = false;
glm::vec3 dayLightColor[8] = {glm::vec3(0.5f, 0.3f, 0.2f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(.8f, .8f, .8f), glm::vec3(0.4f, 0.3f, 0.2f), glm::vec3(0.05f, 0.05f, 0.1f), glm::vec3(0.05f, 0.05f, 0.1f), glm::vec3(0.05f, 0.05f, 0.1f) };

CubeMap* daySky;
CubeMap* dawnSky;
CubeMap* nightSky;
glm::vec3 skyRatio = glm::vec3(1.f, 0.f, 0.f);
glm::vec3 dayLightSky[8] = { glm::vec3(0.5f, 0.f, 0.5f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.8f, 0.2f, 0.f), glm::vec3(0.0f, 1.f, 0.0f), glm::vec3(0.0f, 0.7f, 0.3f), glm::vec3(0.0f, 0.0f, 1.f), glm::vec3(0.0f, 0.0f, 1.f) };

//BoundingVolumeHierarchy* boundingTree;
bool drawTree = false;

//Octree *octTree;
bool drawOctTree = false;

//BSPTree* bspTree;
bool drawBSPTree = false;

static void scrollCallback(GLFWwindow* window, double x, double y)
{
  camera->ScrollInput(x, y);
}

static void mousePosCallback(GLFWwindow* window, double xPos, double yPos)
{
  camera->MouseInput((float)xPos, (float)yPos);
}

static void mouseKeyCallback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
  {
    bool valid;
    Ray r = camera->MouseRayToWorld(valid);

    if (valid)
    {
      Renderer::DebugLineCube(Transform(r.Origin(), glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f)));
    }
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
  {
    camera->SetMouseLook(true);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
  else
  {
    camera->SetMouseLook(false);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

static void updateCycle()
{
}

static void keyboardCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
}

void Scene1::ProcessInput()
{
  if (camera)
  {
    camera->KeyInput(Window::GetWindowHandler());
    camera->UpdateMatrix();
  }
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
Scene1::~Scene1()
{
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
Scene1::Scene1() : Scene()
{
    initMembers();

    //Window::SetKeyCallback(keyboardCallback);
    Window::SetMouseButtonCallback(mouseKeyCallback);
    Window::SetCursorPosCallback(mousePosCallback);
    //Window::SetScrollCallback(scrollCallback);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-unused-return-value"
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void Scene1::initMembers()
{

}
#pragma clang diagnostic pop

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void Scene1::SetupModels()
{
  
  return;
}

void CreateGeometryPBR(const char *name, const glm::vec3 &color, float metallic, float roughness, const glm::vec3 &F0, const char *diffMap, const char *normMap, const char *rmaMap)
{
  Material* newMat = new Material(ResourceManager::Get<Shader>("Geometry PBR"));
  newMat->SetVec3("property.albedo", color);
  newMat->SetVec3("property.F0", F0);
  newMat->SetFloat("property.metallic", metallic);
  newMat->SetFloat("property.roughness", roughness);
  newMat->SetFloat("property.ao", 1.f);
  
  if (diffMap)
  {
    newMat->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>(diffMap));
  }
  else
  {
    newMat->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>("white"));
  }
  if (normMap)
  {
    newMat->SetSampler2D("property.normMap", ResourceManager::Get<Texture>(normMap));
    newMat->SetBool("property.haveNormMap", true);
  }
  else
  {
    newMat->SetSampler2D("property.normMap", ResourceManager::Get<Texture>("black"));
    newMat->SetBool("property.haveNormMap", false);
  }
  if (rmaMap)
  {
    newMat->SetSampler2D("property.RMAMap", ResourceManager::Get<Texture>(rmaMap));
  }
  else
  {
    newMat->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>("white"));
  }

  ResourceManager::Add<Material>(newMat, name);
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
int Scene1::Init()
{
#ifdef COLOR_STOCHASTIC_SHADOW
    ShaderParser::GlobalDefine("COLOR_SHADOW");
#else
    
#endif

    ResourceManager::Init();

    TextureLoader::Init(ResourceManager::Get<Shader>("Hdr To Cube"), ResourceManager::Get<Shader>("Convolute Cube"), ResourceManager::Get<Shader>("Prefilter Cube"), 
                        ResourceManager::Get<Shader>("Gen BRDF"), ResourceManager::Get<Model>("cube"));

    Coordinator::Init();
    Coordinator::RegisterComponent<Transform>();
    Coordinator::RegisterComponent<MeshComp>();
    Coordinator::RegisterComponent<SkinnedMeshComp>();
    Coordinator::RegisterComponent<SkeletalAnimationSet>();
    Coordinator::RegisterComponent<BSpline>();
    Coordinator::RegisterComponent<IKAnimation>();
    Coordinator::RegisterComponent<ClothComp>();
    Coordinator::RegisterComponent<Movement>();
    Coordinator::RegisterComponent<Mass>();
    Coordinator::RegisterComponent<ColliderBox>();
    Coordinator::RegisterComponent<ColliderSphere>();
    Coordinator::RegisterComponent<TestComp>();
    Coordinator::RegisterComponent<AudioSource>();
    Coordinator::RegisterComponent<FuzzyAI>();

    Signature sign;

    meshSystem = Coordinator::RegisterSystem<MeshSystem>();
    sign.set(Coordinator::GetComponentType<Transform>());
    sign.set(Coordinator::GetComponentType<MeshComp>());
    Coordinator::SetSystemSignature<MeshSystem>(sign);
    sign.reset();

    skinnedMeshSystem = Coordinator::RegisterSystem<SkinnedMeshSystem>();
    sign.set(Coordinator::GetComponentType<Transform>());
    sign.set(Coordinator::GetComponentType<SkinnedMeshComp>());
    Coordinator::SetSystemSignature<SkinnedMeshSystem>(sign);
    sign.reset();

    skeletalAnimatorSystem = Coordinator::RegisterSystem<SkeletalAnimator>();
    sign.set(Coordinator::GetComponentType<SkinnedMeshComp>());
    sign.set(Coordinator::GetComponentType<SkeletalAnimationSet>());
    Coordinator::SetSystemSignature<SkeletalAnimator>(sign);
    sign.reset();

    splinePathSystem = Coordinator::RegisterSystem<SplinePathSystem>();
    sign.set(Coordinator::GetComponentType<Transform>());
    sign.set(Coordinator::GetComponentType<BSpline>());
    Coordinator::SetSystemSignature<SplinePathSystem>(sign);
    sign.reset();

    ikSystem = Coordinator::RegisterSystem<IKSystem>();
    sign.set(Coordinator::GetComponentType<Transform>());
    sign.set(Coordinator::GetComponentType<SkinnedMeshComp>());
    sign.set(Coordinator::GetComponentType<IKAnimation>());
    Coordinator::SetSystemSignature<IKSystem>(sign);
    sign.reset();

    clothSystem = Coordinator::RegisterSystem<ClothSystem>();
    sign.set(Coordinator::GetComponentType<ClothComp>());
    Coordinator::SetSystemSignature<ClothSystem>(sign);
    sign.reset();

    boxSys = Coordinator::RegisterSystem<ColliderBoxSystem>();
    sign.set(Coordinator::GetComponentType<Transform>());
    sign.set(Coordinator::GetComponentType<ColliderBox>());
    Coordinator::SetSystemSignature<ColliderBoxSystem>(sign);
    sign.reset();

    sphereSys = Coordinator::RegisterSystem<ColliderSphereSystem>();
    sign.set(Coordinator::GetComponentType<Transform>());
    sign.set(Coordinator::GetComponentType<ColliderSphere>());
    Coordinator::SetSystemSignature<ColliderSphereSystem>(sign);
    sign.reset();

    audioSrcSys = Coordinator::RegisterSystem<AudioSourceSystem>();
    sign.set(Coordinator::GetComponentType<AudioSource>());
    Coordinator::SetSystemSignature<AudioSourceSystem>(sign);
    sign.reset();

    fuzzyAISys = Coordinator::RegisterSystem<FuzzyAISystem>();
    sign.set(Coordinator::GetComponentType<Transform>());
    sign.set(Coordinator::GetComponentType<FuzzyAI>());
    Coordinator::SetSystemSignature<FuzzyAISystem>(sign);
    sign.reset();
    
    /*physics = Coordinator::RegisterSystem<Physics>();
    sign.set(Coordinator::GetComponentType<Transform>());
    sign.set(Coordinator::GetComponentType<Movement>());
    Coordinator::SetSystemSignature<Physics>(sign);
    sign.reset();*/

    gravity = Coordinator::RegisterSystem<Gravity>();
    sign.set(Coordinator::GetComponentType<Mass>());
    sign.set(Coordinator::GetComponentType<Movement>());
    Coordinator::SetSystemSignature<Gravity>(sign);
    sign.reset();

    /*collisionSolver = Coordinator::RegisterSystem<CollisionSolver>();
    sign.set(Coordinator::GetComponentType<Transform>());
    sign.set(Coordinator::GetComponentType<Collider>());
    Coordinator::SetSystemSignature<CollisionSolver>(sign);
    sign.reset();*/

    testsys = Coordinator::RegisterSystem<TestSystem>();
    sign.set(Coordinator::GetComponentType<Transform>());
    sign.set(Coordinator::GetComponentType<TestComp>());
    Coordinator::SetSystemSignature<TestSystem>(sign);
    sign.reset();

    // Create and compile our GLSL program from the shaders
    normalLineShader = new Shader("Normal Line", "../Common/shaders/NormalLine.vert", "../Common/shaders/NormalLine.frag");
    depthMapShader = new Shader("Depth Map", "../Common/shaders/DepthMap.vert", "../Common/shaders/DepthMap.frag");
    skyboxShader = new Shader("Skybox", "../Common/shaders/Skybox.vert", "../Common/shaders/Skybox.frag");
    gBufferShader = new Shader("G-buffer", "../Common/shaders/Gbuffer.vert", "../Common/shaders/Gbuffer.frag");
    lightPassShader = new Shader("Light Pass", "../Common/shaders/Lightpass.vert", "../Common/shaders/Lightpass.frag");
    gBufferSkinnedShader = new Shader("G-buffer-skinned", "../Common/shaders/Gbuffer-Skinned.vert", "../Common/shaders/Gbuffer.frag");
    Shader *defShader = new Shader("Default", "../Common/shaders/Default.vert", "../Common/shaders/Default.frag");

    /*shaders.push_back(new Shader("Phong Lighting", "../Common/shaders/PhongLighting.vert", "../Common/shaders/PhongLighting.frag"));
    shaders.push_back(new Shader("Phong Shading", "../Common/shaders/PhongShadow.vert", "../Common/shaders/PhongShadow.frag"));
    shaders.push_back(new Shader("Blinn Shading", "../Common/shaders/PhongBlinnShadow.vert", "../Common/shaders/PhongBlinnShadow.frag"));*/
    //shaders.push_back(new Shader("Unlit", "../Common/shaders/Unlit.vert", "../Common/shaders/Unlit.frag"));
    shaders.push_back(new Shader("Normal", "../Common/shaders/Normal.vert", "../Common/shaders/Normal.frag"));
    shaders.push_back(new Shader("Depth", "../Common/shaders/Depth.vert", "../Common/shaders/Depth.frag"));
    //mainShader = shaders[2];

    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/ClusterPhongAlpha.shader", "Cluster Phong Alpha"), "Cluster Phong Alpha");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/ClusterPBRAlpha.shader", "Cluster PBR Alpha"), "Cluster PBR Alpha");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/SkyboxCubemap.shader", "Skybox Cubemap"), "Skybox Cubemap");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/OITCopyOpaque.shader", "OIT Copy Opaque"), "OIT Copy Opaque");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/CombineOIT.shader", "Combine OIT"), "Combine OIT");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/DrawTransparentShadow.shader", "Draw Transparent Shadow"), "Draw Transparent Shadow");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/DepthQuad.shader", "Depth Quad"), "Depth Quad");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/DrawShadow.shader", "Draw Shadow"), "Draw Shadow");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/DrawCubeShadow.shader", "Draw Cube Shadow"), "Draw Cube Shadow");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/GaussianFilter.shader", "Gaussian Filter"), "Gaussian Filter");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/Gaussian5x5.shader", "Gaussian 5x5"), "Gaussian 5x5");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/Gaussian9x9.shader", "Gaussian 9x9"), "Gaussian 9x9");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/StochasticToVSM.shader", "Stochastic to VSM"), "Stochastic to VSM");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/ClusterDeferredGeometry.shader", "Clustered Geometry"), "Clustered Geometry");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/ClusterDeferredLightpass.shader", "Clustered Lightpass"), "Clustered Lightpass");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/ClusteredLightpassPBR.shader", "Lightpass PBR"), "Lightpass PBR");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/BloomFilter.shader", "Bloom Filter"), "Bloom Filter");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/BloomCombine.shader", "Bloom Combine"), "Bloom Combine");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/VSMCopy.shader", "VSM Copy"), "VSM Copy");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/SSAO.shader", "SSAO"), "SSAO");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/SSAOBlur.shader", "SSAO Blur"), "SSAO Blur");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/SSR.shader", "SSR"), "SSR");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/ColorCorrection.shader", "Color Correction"), "Color Correction");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/KawaseBlurUp.shader", "Kawase Up"), "Kawase Up");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/KawaseBlurDown.shader", "Kawase Down"), "Kawase Down");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/MomentShadow.shader", "Moment Shadow"), "Moment Shadow");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/MomentBlur.shader", "Moment Blur"), "Moment Blur");
    ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/TextureQuadDepth.shader", "TextureQuadDepth"), "TextureQuadDepth");
    
    ResourceManager::Add<Shader>(defShader, defShader->GetName().c_str());
    ResourceManager::Add<Shader>(normalLineShader, normalLineShader->GetName().c_str());
    ResourceManager::Add<Shader>(depthMapShader, depthMapShader->GetName().c_str());
    ResourceManager::Add<Shader>(skyboxShader, skyboxShader->GetName().c_str());
    ResourceManager::Add<Shader>(new Shader("Unlit", "../Common/shaders/Unlit.vert", "../Common/shaders/Unlit.frag"), "Unlit");
    //ResourceManager::Add<Shader>(new Shader("Skybox Cubemap", "../Common/shaders/SkyboxCubemap.vert", "../Common/shaders/SkyboxCubemap.frag"), "Skybox Cubemap");
    ResourceManager::Add<Shader>(new Shader("G-buffer Position", "../Common/shaders/GbufferPosition.vert", "../Common/shaders/GbufferPosition.frag"), "G-buffer Position");
    ResourceManager::Add<Shader>(new Shader("G-buffer Normal", "../Common/shaders/GbufferNormal.vert", "../Common/shaders/GbufferNormal.frag"), "G-buffer Normal");
    ResourceManager::Add<Shader>(new Shader("G-buffer Ambient", "../Common/shaders/GbufferAmbient.vert", "../Common/shaders/GbufferAmbient.frag"), "G-buffer Ambient");
    ResourceManager::Add<Shader>(new Shader("G-buffer Diffuse", "../Common/shaders/GbufferDiffuse.vert", "../Common/shaders/GbufferDiffuse.frag"), "G-buffer Diffuse");
    ResourceManager::Add<Shader>(new Shader("G-buffer Specular", "../Common/shaders/GbufferSpecular.vert", "../Common/shaders/GbufferSpecular.frag"), "G-buffer Specular");
    ResourceManager::Add<Shader>(new Shader("G-buffer Shininess", "../Common/shaders/GbufferShininess.vert", "../Common/shaders/GbufferShininess.frag"), "G-buffer Shininess");
    ResourceManager::Add<Shader>(new Shader("G-buffer Shadow", "../Common/shaders/GbufferShadow.vert", "../Common/shaders/GbufferShadow.frag"), "G-buffer Shadow");
    ResourceManager::Add<Shader>(new Shader("Shadow Atlas", "../Common/shaders/ShadowAtlas.vert", "../Common/shaders/ShadowAtlas.frag"), "shadow atlas");
    ResourceManager::Add<Shader>(new Shader("Cluster visual", "../Common/shaders/ClusterVisual.vert", "../Common/shaders/ClusterVisual.frag"), "Cluster visual");
    //ResourceManager::Add<Shader>(new Shader("Cluster Phong Alpha", "../Common/shaders/ClusterPhong.vert", "../Common/shaders/ClusterPhongAlpha.frag"), "Cluster Phong Alpha");

    ResourceManager::Add<Texture>(new Texture("../Common/textures/bluenoise.png", TT_2D, TEB_Wrap, TAB_Linear), "uniform noise");
    ResourceManager::Add<Texture>(TextureLoader::GenerateBRDF(), "BRDF");
    /*ResourceManager::Add<Texture>(new Texture("../Common/textures/rock_diffuse.png", TT_2D, TEB_Wrap, TAB_Linear), "rock_diff");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/rock_normal.png", TT_2D, TEB_Wrap, TAB_Linear), "rock_norm");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/rock_specular.png", TT_2D, TEB_Wrap, TAB_Linear), "rock_spec");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/stone_diffuse.png", TT_2D, TEB_Wrap, TAB_Linear), "stone_diff");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/stone_normal.png", TT_2D, TEB_Wrap, TAB_Linear), "stone_norm");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/stone_specular.png", TT_2D, TEB_Wrap, TAB_Linear), "stone_spec");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/dirt_diffuse.png", TT_2D, TEB_Wrap, TAB_Linear), "dirt_diff");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/dirt_normal.png", TT_2D, TEB_Wrap, TAB_Linear), "dirt_norm");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/dirt_specular.png", TT_2D, TEB_Wrap, TAB_Linear), "dirt_spec");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/cliff_diffuse.png", TT_2D, TEB_Wrap, TAB_Linear), "cliff_diff");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/cliff_normal.png", TT_2D, TEB_Wrap, TAB_Linear), "cliff_norm");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/cliff_specular.png", TT_2D, TEB_Wrap, TAB_Linear), "cliff_spec");*/
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Gravel020_1K_Color.jpg", TT_2D, TEB_Wrap, TAB_Linear), "gravel color");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Gravel020_1K_Normal.jpg", TT_2D, TEB_Wrap, TAB_Linear), "gravel normal");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Gravel020_1K_RMA.png", TT_2D, TEB_Wrap, TAB_Linear), "gravel rma");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Ground037_1K_Color.jpg", TT_2D, TEB_Wrap, TAB_Linear), "grass color");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Ground037_1K_Normal.jpg", TT_2D, TEB_Wrap, TAB_Linear), "grass normal");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Ground037_1K_RMA.png", TT_2D, TEB_Wrap, TAB_Linear), "grass rma");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Ground040_1K_RMA.png", TT_2D, TEB_Wrap, TAB_Linear), "maple rma");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Ground040_1K_Color.jpg", TT_2D, TEB_Wrap, TAB_Linear), "maple color");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Ground040_1K_Normal.jpg", TT_2D, TEB_Wrap, TAB_Linear), "maple normal");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Metal035_1K_Normal.jpg", TT_2D, TEB_Wrap, TAB_Linear), "metal normal");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Metal035_1K_RMA.png", TT_2D, TEB_Wrap, TAB_Linear), "metal rma");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/PaintedMetal013_1K_Color.jpg", TT_2D, TEB_Wrap, TAB_Linear), "painted color");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/PaintedMetal013_1K_Normal.jpg", TT_2D, TEB_Wrap, TAB_Linear), "painted normal");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/PaintedMetal013_1K_RMA.png", TT_2D, TEB_Wrap, TAB_Linear), "painted rma");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Wood049_1K_Color.jpg", TT_2D, TEB_Wrap, TAB_Linear), "wood color");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Wood049_1K_Normal.jpg", TT_2D, TEB_Wrap, TAB_Linear), "wood normal");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/pbr/Wood049_1K_RMA.png", TT_2D, TEB_Wrap, TAB_Linear), "wood rma");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/odt.png", TT_2D, TEB_Wrap, TAB_Linear), "odt");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/odtbox.png", TT_2D, TEB_Wrap, TAB_Linear), "odtbox");
    ResourceManager::Add<Texture>(new Texture("../Common/textures/oakleaf.png", TT_2D, TEB_Wrap, TAB_Linear), "leaf");

    ResourceManager::Add<Model>(LoadModelFile("../Common/models/bunny.obj", NormalBaseMode::NBM_Vertex), "bunny");
    ResourceManager::Add<Model>(LoadModelFile("../Common/models/sphere.obj", NormalBaseMode::NBM_Vertex), "load sphere");
    ResourceManager::Add<Model>(ReadObjFile("../Common/models/odtbox.obj"), "odtbox");
    ResourceManager::Add<Model>(ReadObjFile("../Common/models/leavedoc1.obj"), "leaf");

    /*daySky = new CubeMap("../Common/textures/skybox/sky", TEB_Wrap, TAB_Linear);
    nightSky = new CubeMap("../Common/textures/skybox/night", TEB_Wrap, TAB_Linear);
    dawnSky = new CubeMap("../Common/textures/skybox/dawn", TEB_Wrap, TAB_Linear);*/

    ResourceManager::Add<CubeMap>(new CubeMap("../Common/textures/skybox/dawn", TEB_Wrap, TAB_Linear), "Dawn Sky");
    ResourceManager::Add<CubeMap>(new CubeMap("../Common/textures/skybox/sky", TEB_Wrap, TAB_Linear), "Day Sky");
    ResourceManager::Add<CubeMap>(new CubeMap("../Common/textures/skybox/night", TEB_Wrap, TAB_Linear), "Night Sky");
    ResourceManager::Add<CubeMap>(TextureLoader::LoadHDR("../Common/textures/hdr/venice_sunset_2k.hdr"), "hdr cube");
    ResourceManager::Add<CubeMap>(TextureLoader::ConvoluteCubemap(ResourceManager::Get<CubeMap>("hdr cube")), "irridiance map");
    ResourceManager::Add<CubeMap>(TextureLoader::PrefilterCubemap(ResourceManager::Get<CubeMap>("hdr cube")), "prefilter map");

    int hdrW, hdrH;
    float* rawHDR = TextureLoader::LoadHDRRaw("../Common/textures/hdr/venice_sunset_2k.hdr", hdrW, hdrH);
    Renderer::SetSphericalHarmonics(rawHDR, hdrW, hdrH);
    TextureLoader::FreeSTBIImage(rawHDR);
    
    Material* skyCube = new Material(ResourceManager::Get<Shader>("Skybox Cubemap"));
    skyCube->SetSamplerCube("property.skyTex", ResourceManager::Get<CubeMap>("hdr cube"));
    ResourceManager::Add<Material>(skyCube, "Skybox Cubemap");

#ifdef DEFERRED_SHADING
    const char* defaulShader = "Geometry Basic";
#else
    const char* defaulShader = "Cluster Phong";
#endif

    Material* clustAlpha = new Material(ResourceManager::Get<Shader>("Cluster PBR Alpha"));
    clustAlpha->SetBlendMode(ShaderConfig::BLEND_Translucent);
    clustAlpha->SetVec3("property.albedo", glm::vec3(0.3f, 0.7f, 1.f));
    clustAlpha->SetFloat("property.etaRatio", 0.8f);
    clustAlpha->SetFloat("property.metallic", 1.0f);
    clustAlpha->SetFloat("property.roughness", 0.0f);
    clustAlpha->SetFloat("property.alpha", 1.0f);
    clustAlpha->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>("white"));
    clustAlpha->SetSampler2D("property.normMap", ResourceManager::Get<Texture>("black"));
    clustAlpha->SetSampler2D("property.RMAMap", ResourceManager::Get<Texture>("white"));
    ResourceManager::Add<Material>(clustAlpha, "alpha");

    clustAlpha = new Material(ResourceManager::Get<Shader>("Cluster PBR Alpha"));
    clustAlpha->SetBlendMode(ShaderConfig::BLEND_Translucent);
    clustAlpha->SetVec3("property.albedo", glm::vec3(0.84f, 0.38f, 0.2f));
    clustAlpha->SetFloat("property.etaRatio", 0.8f);
    clustAlpha->SetFloat("property.metallic", 1.0f);
    clustAlpha->SetFloat("property.roughness", 0.0f);
    clustAlpha->SetFloat("property.alpha", 1.0f);
    clustAlpha->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>("white"));
    clustAlpha->SetSampler2D("property.normMap", ResourceManager::Get<Texture>("black"));
    clustAlpha->SetSampler2D("property.RMAMap", ResourceManager::Get<Texture>("white"));
    ResourceManager::Add<Material>(clustAlpha, "alpha2");

    clustAlpha = new Material(ResourceManager::Get<Shader>("Cluster PBR Alpha"));
    clustAlpha->SetBlendMode(ShaderConfig::BLEND_Translucent);
    clustAlpha->SetVec3("property.albedo", glm::vec3(0.58f, 0.83f, 0.47f));
    clustAlpha->SetFloat("property.etaRatio", 0.8f);
    clustAlpha->SetFloat("property.metallic", 1.0f);
    clustAlpha->SetFloat("property.roughness", 0.0f);
    clustAlpha->SetFloat("property.alpha", 1.0f);
    clustAlpha->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>("white"));
    clustAlpha->SetSampler2D("property.normMap", ResourceManager::Get<Texture>("black"));
    clustAlpha->SetSampler2D("property.RMAMap", ResourceManager::Get<Texture>("white"));
    ResourceManager::Add<Material>(clustAlpha, "alpha3");

    Material* odt = new Material(ResourceManager::Get<Shader>("Cluster PBR Alpha"));
    odt->SetBlendMode(ShaderConfig::BLEND_Translucent);
    odt->SetVec3("property.albedo", glm::vec3(1.f, 0.5f, 0.f));
    odt->SetFloat("property.transmission", 0.f);
    odt->SetFloat("property.alpha", 0.9f);
    odt->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>("odt"));
    odt->SetSampler2D("property.normMap", ResourceManager::Get<Texture>("black"));
    odt->SetSampler2D("property.RMAMap", ResourceManager::Get<Texture>("white"));
    ResourceManager::Add<Material>(odt, "odt0");

    odt = new Material(ResourceManager::Get<Shader>("Cluster PBR Alpha"));
    odt->SetBlendMode(ShaderConfig::BLEND_Translucent);
    odt->SetVec3("property.albedo", glm::vec3(1.f));
    odt->SetFloat("property.transmission", 0.f);
    odt->SetFloat("property.alpha", 0.9f);
    odt->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>("odtbox"));
    odt->SetSampler2D("property.normMap", ResourceManager::Get<Texture>("black"));
    odt->SetSampler2D("property.RMAMap", ResourceManager::Get<Texture>("white"));
    ResourceManager::Add<Material>(odt, "odtbox");

    Material* odt1 = new Material(ResourceManager::Get<Shader>("Cluster PBR Alpha"));
    odt1->SetBlendMode(ShaderConfig::BLEND_Translucent);
    odt1->SetVec3("property.albedo", glm::vec3(0.f, 1.0f, 0.5f));
    odt1->SetFloat("property.transmission", 0.f);
    odt1->SetFloat("property.alpha", 0.9f);
    odt1->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>("odt"));
    odt1->SetSampler2D("property.normMap", ResourceManager::Get<Texture>("black"));
    odt1->SetSampler2D("property.RMAMap", ResourceManager::Get<Texture>("white"));
    ResourceManager::Add<Material>(odt1, "odt1");

    Material* leaf = new Material(ResourceManager::Get<Shader>("Cluster PBR Alpha"));
    leaf->SetBlendMode(ShaderConfig::BLEND_Translucent);
    leaf->SetVec3("property.albedo", glm::vec3(0.3f, 1.0f, 0.2f));
    leaf->SetFloat("property.transmission", 0.f);
    leaf->SetFloat("property.alpha", 1.0f);
    leaf->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>("leaf"));
    leaf->SetSampler2D("property.normMap", ResourceManager::Get<Texture>("black"));
    leaf->SetSampler2D("property.RMAMap", ResourceManager::Get<Texture>("white"));
    ResourceManager::Add<Material>(leaf, "leaf");
    
    CreateGeometryPBR("gravel", glm::vec3(0.6f), 0.f, 1.f, glm::vec3(0.04f), "gravel color", "gravel normal", "gravel rma");
    CreateGeometryPBR("grass", glm::vec3(0.6f), 0.f, 1.f, glm::vec3(0.04f), "grass color", "grass normal", "grass rma");
    CreateGeometryPBR("maple", glm::vec3(0.6f), 0.f, 1.f, glm::vec3(0.04f), "maple color", "maple normal", "maple rma");
    CreateGeometryPBR("metal", glm::vec3(1.f,1.f,1.f), 1.f, 0.f, glm::vec3(0.04f), nullptr, "metal normal", "metal rma");
    CreateGeometryPBR("painted", glm::vec3(1.f), 1.f, 0.f, glm::vec3(0.04f), "painted color", "painted normal", "painted rma");
    CreateGeometryPBR("ground", glm::vec3(0.6f), 0.f, 1.f, glm::vec3(0.04f), nullptr, nullptr, nullptr);
    CreateGeometryPBR("sphere0", glm::vec3(0.6f), 0.f, 1.f, glm::vec3(0.04f), nullptr, nullptr, nullptr);
    CreateGeometryPBR("sphere1", glm::vec3(0.6f), 0.3f, 0.7f, glm::vec3(0.04f), nullptr, nullptr, nullptr);
    CreateGeometryPBR("sphere2", glm::vec3(0.6f), 0.5f, 0.5f, glm::vec3(0.04f), nullptr, nullptr, nullptr);
    CreateGeometryPBR("sphere3", glm::vec3(0.6f), 0.7f, 0.3f, glm::vec3(0.04f), nullptr, nullptr, nullptr);
    CreateGeometryPBR("sphere4", glm::vec3(0.6f), 1.f, 0.f, glm::vec3(0.04f), nullptr, nullptr, nullptr);

    Material* mainMat = new Material(ResourceManager::Get<Shader>(defaulShader));
    mainMat->SetVec3("property.diffuse", glm::vec3(1.f));
    mainMat->SetVec3("property.ambient", glm::vec3(0.1f));
    mainMat->SetVec3("property.specular", glm::vec3(1.f));
    mainMat->SetFloat("property.shininess", 16.f);
    //mainMat->SetReflection(.2f);
    mainMat->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>("default"));
    ResourceManager::Add<Material>(mainMat, "main");

    //ResourceManager::Add<Model>(LoadModelFile("../Common/models/g1.ply", NBM_Vertex), "main");
    ResourceManager::Add<SkinnedMesh>(ReadAssimpFile("../Common/models/gh_sample_animation.fbx"), "main");

#ifdef EDITOR_MODE
    unsigned int renderWidth = static_cast<unsigned int>(Window::Width() * 0.6f);
    unsigned int renderHeight = static_cast<unsigned int>(Window::Height() * 0.6f);
#else
    unsigned int renderWidth = static_cast<unsigned int>(Window::Width());
    unsigned int renderHeight = static_cast<unsigned int>(Window::Height());
#endif

    camera = new Camera("Navigate Camera", renderWidth, renderHeight, 0.1f, 500.f, false, glm::vec3(3.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
    Renderer::SetCamera(camera);
    Renderer::SetClearColor(glm::vec3(0.1, 0.3, 0.5));

    Renderer::Init(meshSystem, skinnedMeshSystem, renderWidth, renderHeight, 1000);
    Renderer::SetClothSystem(clothSystem);
    gBufferShader->BindToUniformBlock("Matrices", 0);
    depthMapShader->BindToUniformBlock("Matrices", 0);
    normalLineShader->BindToUniformBlock("Matrices", 0);
    lightPassShader->BindToUniformBlock("Lights", 1);

    Material::SetPreviewMesh(ResourceManager::Get<Model>("sphere"));

    updateCycle();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    ColliderManager::Init(boxSys, sphereSys);

    //EntityID mainModel = Coordinator::CreateEntity();
    ////ClothComp mainCloth(2.f, 16, 1.f, 0.f, 255.f, 255.f, 25.f, 25.f, 0.1f);
    ////mainCloth.material = ResourceManager::Get<Material>("alpha");
    ////Coordinator::AddComponent(mainModel, mainCloth);
    //SkinnedMesh* mainMesh = ResourceManager::Get<SkinnedMesh>("main");
    //Coordinator::AddComponent(mainModel, Transform(glm::vec3(0.f, -1.f, 0.f),  glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.01f, 0.01f, 0.01f), 0.f));
    //Coordinator::AddComponent(mainModel, SkinnedMeshComp(mainMesh, ResourceManager::Get<Material>("default")));
    ////Coordinator::AddComponent(mainModel, IKAnimation(10));
    //SkeletalAnimationSet anim = SkeletalAnimationSet(mainMesh, mainMesh->GetAnimations());
    //anim.SetAnimation(2);
    //Coordinator::AddComponent(mainModel, anim);
    //Coordinator::AddComponent(mainModel, BSpline(100));

    EntityID sphereEnt = Coordinator::CreateEntity("bunny");
    Coordinator::AddComponent(sphereEnt, Transform(glm::vec3(-4.f, 0.f, 4.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(3.f, 3.f, 3.f)));
    Coordinator::AddComponent(sphereEnt, MeshComp(ResourceManager::Get<Model>("bunny"), ResourceManager::Get<Material>("alpha")));
    Coordinator::AddComponent(sphereEnt, ColliderSphere(0.2f));

    EntityID leafent = Coordinator::CreateEntity("leaf");
    Coordinator::AddComponent(leafent, Transform(glm::vec3(-10.f, -3.f, 4.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, 1.f, 1.f)));
    Coordinator::AddComponent(leafent, MeshComp(ResourceManager::Get<Model>("leaf"), ResourceManager::Get<Material>("leaf")));
    Coordinator::AddComponent(leafent, ColliderSphere(0.2f));
    //FuzzyAI bunnyAI(0.f, 4.f, &camera->GetPosition(), 4, 40);
    //
    //bunnyAI.AddAntecedents(0, new TrapezoidFuzzyMember(2,0,true)); // very close
    //bunnyAI.AddAntecedents(1, new TriangularFuzzyMember(0,2,6)); // close
    //bunnyAI.AddAntecedents(2, new TriangularFuzzyMember(2,6,10)); // medium
    //bunnyAI.AddAntecedents(3, new TrapezoidFuzzyMember(6,10,false)); // far

    //bunnyAI.AddConsequences(0, new TrapezoidFuzzyMember(2, 4, false)); // super fast
    //bunnyAI.AddConsequences(1, new TriangularFuzzyMember(0.5, 2, 4)); // fast
    //bunnyAI.AddConsequences(2, new TriangularFuzzyMember(0, 0.5, 2)); // normal
    //bunnyAI.AddConsequences(3, new TrapezoidFuzzyMember(0.5, 0, true)); // slow
    //
    //Coordinator::AddComponent(sphereEnt, bunnyAI);

    EntityID floorEnt = Coordinator::CreateEntity("floor");
    Coordinator::AddComponent(floorEnt, Transform(glm::vec3(0.f, -2.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(25.f, 0.5f, 25.f)));
    MeshComp floorMesh = MeshComp(ResourceManager::Get<Model>("default"), ResourceManager::Get<Material>("grass"));
    floorMesh.boundingVolume.ChangeAABB(floorMesh.BB.BoxVertices);
    Coordinator::AddComponent(floorEnt, floorMesh);

    EntityID odtEnt = Coordinator::CreateEntity("odt box");
    Coordinator::AddComponent(odtEnt, Transform(glm::vec3(-6.f, 2.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(3.f, 3.f, 3.f), 45.f));
    Coordinator::AddComponent(odtEnt, MeshComp(ResourceManager::Get<Model>("default"), ResourceManager::Get<Material>("odt0")));
    //odtEnt = Coordinator::CreateEntity();
    //Coordinator::AddComponent(odtEnt, Transform(glm::vec3(0.f, 2.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(5.f, 5.f, 5.f), 90.f));
    //Coordinator::AddComponent(odtEnt, MeshComp(ResourceManager::Get<Model>("quad"), ResourceManager::Get<Material>("odt1")));

    EntityID odtEnt2 = Coordinator::CreateEntity("odt 1");
    Coordinator::AddComponent(odtEnt2, Transform(glm::vec3(-6.f, 2.f, -4.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(3.f, 3.f, 3.f), -45.f));
    Coordinator::AddComponent(odtEnt2, MeshComp(ResourceManager::Get<Model>("quad"), ResourceManager::Get<Material>("odt0")));
    EntityID odtEnt3 = Coordinator::CreateEntity("odt 2");
    Coordinator::AddComponent(odtEnt3, Transform(glm::vec3(-6.f, 2.f, -4.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(3.f, 3.f, 3.f), 45.f));
    Coordinator::AddComponent(odtEnt3, MeshComp(ResourceManager::Get<Model>("quad"), ResourceManager::Get<Material>("odt1")));

    for (int i = 0; i < 5; i++)
    {
      /*std::string etaStr = "eta";

      Material* eta = new Material(ResourceManager::Get<Shader>("Cluster PBR Alpha"));
      eta->SetBlendMode(ShaderConfig::BLEND_Translucent);
      eta->SetVec3("property.albedo", glm::vec3(0.3f, 0.7f, 1.f));
      eta->SetFloat("property.etaRatio", 0.4f + 0.2f * i);
      eta->SetFloat("property.metallic", 1.0f);
      eta->SetFloat("property.roughness", 1.0f);
      eta->SetFloat("property.alpha", 1.0f);
      eta->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>("white"));
      eta->SetSampler2D("property.normMap", ResourceManager::Get<Texture>("black"));
      eta->SetSampler2D("property.RMAMap", ResourceManager::Get<Texture>("white"));
      ResourceManager::Add<Material>(eta, (etaStr + std::to_string(i)).c_str());

      EntityID entt = Coordinator::CreateEntity("eta");
      glm::vec3 pos(2.f * i, 1.f, 2.f);
      glm::vec3 scale(1.6f);
      Coordinator::AddComponent(entt, Transform(pos, glm::vec3(0.f, 1.f, 0.f), scale));
      Coordinator::AddComponent(entt, MeshComp(ResourceManager::Get<Model>("load sphere"), ResourceManager::Get<Material>((etaStr + std::to_string(i)).c_str())));*/

      EntityID entt = Coordinator::CreateEntity("eta");
      glm::vec3 pos(2.f* i, 1.f, 2.f);
      glm::vec3 scale(1.6f);
      Coordinator::AddComponent(entt, Transform(pos, glm::vec3(0.f, 1.f, 0.f), scale));
      Coordinator::AddComponent(entt, MeshComp(ResourceManager::Get<Model>("load sphere"), ResourceManager::Get<Material>(("sphere" + std::to_string(i)).c_str())));
    }

    {
      EntityID entt = Coordinator::CreateEntity("blue");
      glm::vec3 pos(0.f, 3.f, -1.f);
      glm::vec3 scale(MathUtil::RandomRange(2.5f, 3.f));
      Coordinator::AddComponent(entt, Transform(pos, glm::vec3(0.f, 1.f, 0.f), scale));
      Coordinator::AddComponent(entt, MeshComp(ResourceManager::Get<Model>("load sphere"), ResourceManager::Get<Material>("alpha")));

      entt = Coordinator::CreateEntity("red");
      pos = glm::vec3(1.f, 2.3f, -3.f);
      scale = glm::vec3(MathUtil::RandomRange(2.5f, 3.f));
      Coordinator::AddComponent(entt, Transform(pos, glm::vec3(0.f, 1.f, 0.f), scale));
      Coordinator::AddComponent(entt, MeshComp(ResourceManager::Get<Model>("load sphere"), ResourceManager::Get<Material>("alpha2")));

      entt = Coordinator::CreateEntity("green");
      pos = glm::vec3(-1.f, 4.1f, -2.f);
      scale = glm::vec3(MathUtil::RandomRange(2.5f, 3.f));
      Coordinator::AddComponent(entt, Transform(pos, glm::vec3(0.f, 1.f, 0.f), scale));
      Coordinator::AddComponent(entt, MeshComp(ResourceManager::Get<Model>("load sphere"), ResourceManager::Get<Material>("alpha3")));
    }
    /*static const char* sphereMat[3] = { "alpha","alpha2","alpha3" };
    for (int i = 0; i < 10; i++)
    {
      EntityID entt = Coordinator::CreateEntity();
      glm::vec3 pos(MathUtil::RandomRange(-10.f, 10.f), MathUtil::RandomRange(0.f, 5.f), MathUtil::RandomRange(-10.f, 10.f));
      glm::vec3 scale(MathUtil::RandomRange(2.f, 3.f));
      Coordinator::AddComponent(entt, Transform(pos, glm::vec3(0.f, 1.f, 0.f), scale));
      Coordinator::AddComponent(entt, MeshComp(ResourceManager::Get<Model>("load sphere"), ResourceManager::Get<Material>(sphereMat[(int)MathUtil::RandomRange(0.f, 3.f)])));
    }*/

    static const char* boxMat[3] = {"gravel","painted","metal"};
    for (int i = 0; i < 10; i++)
    {
      EntityID entt = Coordinator::CreateEntity("box");
      glm::vec3 scale(MathUtil::RandomRange(1.f, 5.f), MathUtil::RandomRange(2.f, 5.f), MathUtil::RandomRange(1.f, 5.f));
      float posX = MathUtil::RandomRange(-12.f, 12.f);
      float posZ = MathUtil::RandomRange(8.f, 12.f);
      glm::vec3 pos(posX, -2.f + scale.y / 2.f, posZ);
      Coordinator::AddComponent(entt, Transform(pos, glm::vec3(0.f, 1.f, 0.f), scale, MathUtil::RandomRange(0.f, 360.f)));

      MeshComp boxMesh = MeshComp(ResourceManager::Get<Model>("default"), ResourceManager::Get<Material>(boxMat[(int)MathUtil::RandomRange(0.f, 3.f)]));
      boxMesh.boundingVolume.ChangeAABB(boxMesh.BB.BoxVertices);
      Coordinator::AddComponent(entt, boxMesh);
    }

    /*EntityID floorEnt2 = Coordinator::CreateEntity();
    Coordinator::AddComponent(floorEnt2, Transform(glm::vec3(0.f, 2.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(25.f, 0.5f, 25.f)));
    Coordinator::AddComponent(floorEnt2, MeshComp(ResourceManager::Get<Model>("default"), ResourceManager::Get<Material>("default")));*/
    //gBuffer = new Gbuffer(_windowWidth, _windowHeight);
    //boundingTree = new BoundingVolumeHierarchy(BVHMethod::BVH_Bottomup, BVT_AABB);
    //boundingTree->ComputeTree();

    DirectionalLight* sun = new DirectionalLight("direct 1", glm::vec3(0.5f * cos(glm::radians(dLightAngle)), -1.f, 0.5f * sin(glm::radians(dLightAngle))), glm::vec3(1.f), 1.f, true, true, true, 2048);
    sun->SetFrustumSize(12);
    Renderer::AddDirectionalLight(sun);
    //Renderer::AddDirectionalLight(new DirectionalLight("direct 2", glm::vec3(0.5f* cos(glm::radians(90.f)), -1.f, 0.5f* sin(glm::radians(90.f))), glm::vec3(0.7f, 0.7f, 0.8f), true, 256));
    //Renderer::AddPointLight(new PointLight("p shadow", glm::vec3(), 3.f, glm::vec3(0.2f, 0.8f, 1.f), false, 256));
    //for (int i = 0; i < 50; i++)
    //{
    //  glm::vec3 pclr(MathUtil::RandomRange(0.f, 1.f), MathUtil::RandomRange(0.f, 1.f), MathUtil::RandomRange(0.f, 1.f));
    //  std::string pname("p");
    //  pname += std::to_string(i);
    //  Renderer::AddPointLight(new PointLight(pname, glm::vec3(MathUtil::RandomRange(-12.f, 12.f), MathUtil::RandomRange(-2.f, -1.5f), MathUtil::RandomRange(-12.f, 12.f)), 2.5f, pclr, 50.f));
    //  //Renderer::AddPointLight(new PointLight("p1", glm::vec3(-2.f, 0.f, -2.f), 8.f, glm::vec3(0.3f, 0.15f, 0.f)));
    //}

    //octTree = new Octree();
    //bspTree = new BSPTree();

    skinnedMeshSystem->Init();
    ikSystem->Init();
    clothSystem->Init();

#ifdef EDITOR_MODE

    unsigned int centerLeft = (Window::Width() - renderWidth) / 2;
    unsigned int centerTop = (Window::Height() - renderHeight) / 2;
    unsigned int centerRight = centerLeft + renderWidth;
    unsigned int centerBottom = centerTop + renderHeight;

    Renderer::SetViewRect(Rect<unsigned int>(centerLeft, centerTop, centerRight, centerBottom));

    Editor::Init();
    EditorMenu centereditor("Center Editor", Rect<unsigned int>(centerLeft, centerTop, centerRight, centerBottom));
    EditorMenu lefteditor("Left Editor", Rect<unsigned int>(0, 20, centerLeft, centerBottom));
    EditorMenu righteditor("Right Editor", Rect<unsigned int>(centerRight, 20, Window::Width(), centerBottom));
    EditorMenu bottomeditor("Bottom Editor", Rect<unsigned int>(0, centerBottom, Window::Width(), Window::Height()));
    lefteditor.Add("Scene Editor", Renderer::DrawDebugMenu);
    lefteditor.Add("Entity Manager", EntityManager::DrawDebugMenu);
    righteditor.Add("Object Editor", Editor::DrawObjEditor);
    righteditor.Add("Resource Editor", ResourceManager::DrawEditorMenu);
    bottomeditor.Add("Resouce Selector", ResourceManager::DrawDebugMenu);
    centereditor.Add("Scene Display", Renderer::DrawDisplayMenu);

    Editor::AddMenu(centereditor);
    Editor::AddMenu(lefteditor);
    Editor::AddMenu(righteditor);
    Editor::AddMenu(bottomeditor);
#else

    Renderer::SetViewRect(Rect<unsigned int>(0, 0, Renderer::Width(), Renderer::Height()));
#endif

    return Scene::Init();
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void Scene1::CleanUp()
{
  /*delete boundingTree;
  delete octTree;
  delete bspTree;*/

  Renderer::Exit();
  TextureLoader::Exit();
  ResourceManager::Exit();
  Editor::Exit();

  // Cleanup VBO
  delete camera;

  //glDeleteProgram(programID);

  for (unsigned int i = 0; i < shaders.size(); i++)
    delete shaders[i];
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
int Scene1::preRender()
{
  

  return 0;
}

static void RenderDebugObjects(Shader *shader)
{
  normalLineShader->use();
  /*if (drawNormalLine)
  {
    normalLineShader->setVec3("color", glm::vec3(0.f, 0.f, 1.f));
    test->DrawNormal(normalLineShader);
  }*/

  if (drawOrbitLine)
  {
    Renderer::DebugLineCircle(Transform(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(orbitRadius * 2.f, 1.f, orbitRadius * 2.f)), glm::vec3(0.f, 0.f, 1.f));
  }

  splinePathSystem->Draw(shader);
  /*if(drawTree) boundingTree->Draw();

  if (drawOctTree) octTree->Draw();
  if (drawBSPTree) bspTree->Draw();*/
}

void Scene1::Update()
{
  Profiler::Begin("Update");

  ColliderManager::Update();
  skeletalAnimatorSystem->Update();
  ikSystem->Update();

  Profiler::End();

  /*if (orbit)
  {
    orbitRotation += .5f;
    if (orbitRotation > 360.f)
      orbitRotation -= 360.f;
    else if (orbitRotation < 0.f)
      orbitRotation += 360.f;

    if (lightRotateWithOrbit)
    {
      for (int i = 0; i < sphereNum / 2; i++)
      {
        if (i < (int)Renderer::GetPointLightSize())
        {
          float step = ((float)i*2 * 360.f / sphereNum) + orbitRotation;
          step = glm::radians(step);
          glm::vec3 pos(orbitRadius * sin(step), .5f, orbitRadius * cos(step));
          Renderer::GetPointLight(i)->SetPosition(pos);
        }
      }
    }
  }*/

  //test->transform.SetRotationAngle(test->transform.GetRotationAngle() + 0.1f);

  /*if (dLightRotate)
  {
    dLightAngle += 0.1f;
    if (dLightAngle > 360.f)
      dLightAngle -= 360.f;
    else if (dLightAngle < 0.f)
      dLightAngle += 360.f;

    glm::vec3 dir = glm::vec3(0.5f * cos(glm::radians(dLightAngle)), -1.f, 0.5f * sin(glm::radians(dLightAngle)));
    DirectionalLight* sunLight = Renderer::GetDirectionalLight(0);
    sunLight->SetDirection(dir);

    float dayRatio = dLightAngle / 360.f;
    int lightIndexStart = (int)(dayRatio / 0.125f);
    int lightIndexEnd = (lightIndexStart > 6) ? 0 : lightIndexStart + 1;
    float inbetween = (dayRatio - (lightIndexStart * 0.125f)) * 8.f;
    glm::vec3 lightColor = (dayLightColor[lightIndexStart] * (1.f - inbetween)) + (dayLightColor[lightIndexEnd] * inbetween);
    sunLight->SetDiffuseColor(lightColor);
    sunLight->SetSpecularColor(lightColor);
    lightColor *= 0.2f;
    sunLight->SetAmbientColor(lightColor);

    if (Renderer::GetSpotLightSize())
    {
      lightColor = (lightIndexStart > 4) ? glm::vec3(1.f, 0.8f, 0.7f) : glm::vec3(0.f);
      Renderer::GetSpotLight(0)->SetDiffuseColor(lightColor);
    }

    skyRatio = (dayLightSky[lightIndexStart] * (1.f - inbetween)) + (dayLightSky[lightIndexEnd] * inbetween);
  }*/
}

void Scene1::FixedUpdate()
{
  clothSystem->Update();
  testsys->Update();
  gravity->Update();
  fuzzyAISys->Update();
  //physics->Update();
  //collisionSolver->Update();
}

int Scene1::Render()
{
    //glViewport(renderer->PosX(), renderer->PosY(), renderer->Width(), renderer->Height());
    RenderDebugObjects(shaders[0]);
    //glViewport(0, 0, Window::Width(), Window::Height());

    Profiler::Begin("Render");

    Renderer::Update();
    Renderer::Draw();
    Renderer::DrawDebug(normalLineShader);

    Profiler::End();

#ifdef EDITOR_MODE
    Editor::DrawMenus();
#else
    Renderer::DrawFinalToQuad();
    Profiler::DrawDebugMenu();
    Time::DrawDebugMenu();
#endif

    return 0;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
int Scene1::postRender()
{
    return 0;
}

void Scene1::DrawDebugMenu(Rect<unsigned int> window)
{
  static Light* ledit = nullptr;
  static std::string editName;

  ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "NAVIGATE CAMERA:");
  ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Right click + move cursor to look around");
  ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "W/S/A/D to move camera");
  ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Space to ascend camera");
  ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "L Shift to descend camera");
  ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Scroll to zoom in/out camera's fov");
  ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "EDITING OBJECTS:");
  ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Open Renderer -> RenderObjects -> click on object");
  ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "EDITING BOUNDING VOLUME:");
  ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Editing Object -> it is in render object menu");
  ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "EDITING BOUNDING VOLUME HIERARCHY:");
  ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Open Bounding Volumes Hierarchy");
  ImGui::NewLine();

  ImGui::Text("FPS: %f", Time::FPS());
  /*for (unsigned int i = 0; i < shaders.size(); i++)
  {
    Shader* shader = shaders[i];
    std::string id = std::to_string(shader->GetID());
    std::string name = shader->GetName() + " [" + id + ']';

    if (ImGui::Button(name.c_str()))
    {
      mainShader = shader;
    }
    ImGui::SameLine();
    if (ImGui::Button(("Reload " + id).c_str()))
    {
      shader->Reload();
      break;
    }
  }
  ImGui::Text("Current Shader: %s, ID: %i", mainShader->GetName().c_str(), mainShader->GetID());*/

  /*if (ImGui::DragInt("Scene Setup", &lightCycle, 0.1f, 0, 4))
    updateCycle();
  if (ImGui::IsItemHovered())
  {
    ImGui::BeginTooltip();
    ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Switching between pre-arranged setups for the scene to showcase the system");
    ImGui::EndTooltip();
  }*/

  ImGui::Checkbox("Show Normal Lines", &drawNormalLine);
  ImGui::Checkbox("Rotate Orbit", &orbit);
  ImGui::Checkbox("Make Point lights rotate with Orbit", &lightRotateWithOrbit);
  ImGui::Checkbox("Day-Night Cycle", &dLightRotate);

  ImGui::Text("Skybox");
  ImGui::DragFloat("Day Ratio", &skyRatio[0], 0.01f, 0.f, 1.f);
  ImGui::DragFloat("Dawn Ratio", &skyRatio[1], 0.01f, 0.f, 1.f);
  ImGui::DragFloat("Night Ratio", &skyRatio[2], 0.01f, 0.f, 1.f);

  //Renderer::DrawDebugMenu();
  
  /*ImGui::Separator();
  static bool showBV = false;
  if(ImGui::Checkbox("Show Bounding Volumes", &showBV))
  {
    std::vector<RenderObject*>& objects = Renderer::GetObjects();
    for (RenderObject* obj : objects)
    {
      if (obj)
      {
        obj->GetBoundingVolume()->SetHide(!showBV);
      }
    }
  }

  ImGui::Checkbox("Show Bounding Volumes Hierarchy", &drawTree);
  boundingTree->DrawDebugMenu();

  ImGui::Separator();

  ImGui::Checkbox("Show Octree", &drawOctTree);
  octTree->DrawDebugMenu();

  ImGui::Separator();

  ImGui::Checkbox("Show BSP Tree", &drawBSPTree);
  bspTree->DrawDebugMenu();*/
}
