#include "ResourceManager.h"
#include "shader.hpp"
#include "ModelGenerator.h"
#include "ModelLoader.h"
#include "Texture.h"
#include "Log.h"
#include "Editor.h"
#include "SkinnedMesh.h"
#include "AudioClip.h"
#include "Cubemap.h"
#include "ShaderParser.h"
#include "GraphicAPI.h"

Model* ResourceManager::Quad;

//ResourceMap<Texture> ResourceManager::textures;
//ResourceMap<Model> ResourceManager::meshes;
//ResourceMap<Shader> ResourceManager::shaders;
//ResourceMap<Material> ResourceManager::materials;

std::vector<IResourceMap*> ResourceManager::ResourceMaps;
std::unordered_map<const char*, unsigned int> ResourceManager::TypeToID;
unsigned int ResourceManager::NextResourceMapID;
unsigned int ResourceManager::EditorTypeID;

Texture* ResourceManager::editTexture;
Model* ResourceManager::editModel;
Material* ResourceManager::editMaterial;
Shader* ResourceManager::editShader;
SkinnedMesh* ResourceManager::editSkinnedMesh;
std::string ResourceManager::editName;

void ResourceManager::Init()
{
  LOG_TRACE("Resource", "Initializing Resource Manager...");

  NextResourceMapID = 0;
  EditorTypeID = 0;

  RegisterResource<Shader>();
  RegisterResource<Model>();
  RegisterResource<Material>();
  RegisterResource<Texture>();
  RegisterResource<CubeMap>();
  RegisterResource<SkinnedMesh>();
  RegisterResource<AudioClip>();

  Quad = ModelGenerator::CreateQuad();

  GLubyte defaultTex[] = { 200, 200, 200, 255, 100, 100, 100, 255, 100, 100, 100, 255, 200, 200, 200, 255 };
  Add<Texture>(new Texture(defaultTex, 2, 2, TT_2D, 4U), "default");

  Add<Shader>(new Shader("Texture Atlas", "../Common/shaders/TextureAtlas.vert", "../Common/shaders/TextureAtlas.frag"), "texture atlas");
  Add<Shader>(new Shader("Mesh Preview", "../Common/shaders/MeshPreview.vert", "../Common/shaders/MeshPreview.frag"), "mesh preview");
  ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/TextureQuad.shader", "Texture Quad"), "Texture Quad");
  ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/ClusterPhong.shader", "Cluster Phong"), "Cluster Phong");
  ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/GeometryBasic.shader", "Geometry Basic"), "Geometry Basic");
  ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/GeometryPBR.shader", "Geometry PBR"), "Geometry PBR");
  ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/HdrToCube.shader", "Hdr To Cube"), "Hdr To Cube");
  ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/ConvoluteCube.shader", "Convolute Cube"), "Convolute Cube");
  ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/FilterSpecularCube.shader", "Prefilter Cube"), "Prefilter Cube");
  ResourceManager::Add<Shader>(ShaderParser::ReadShader("../Common/shaders/GenBRDF.shader", "Gen BRDF"), "Gen BRDF");

  GLubyte whiteTexp[] = { 255, 255, 255, 255 };
  GLubyte blackTexp[] = { 0, 0, 0, 0 };
  ResourceManager::Add<Texture>(new Texture(whiteTexp, 1, 1, TT_2D, 4U), "white");
  ResourceManager::Add<Texture>(new Texture(blackTexp, 1, 1, TT_2D, 4U), "black");

  ResourceMap<Model>* meshMap = GetResourceMap<Model>();
  meshMap->CreatePreview();
  meshMap->SetPreviewShader(Get<Shader>("mesh preview"));
  meshMap->SetHavePreview(true);
  //meshes.Add(ModelGenerator::CreateCube(), "default");
  //textures.Add(new Texture(), "default");

#ifdef DEFERRED_SHADING
  const char* defaulShader = "Geometry PBR";
#else
  const char* defaulShader = "Cluster Phong";
#endif

  Material* defMat = new Material(Get<Shader>(defaulShader));
  defMat->SetVec3("property.albedo", glm::vec3(0.7f));
  defMat->SetVec3("property.F0", glm::vec3(0.04f));
  defMat->SetFloat("property.metallic", 0.f);
  defMat->SetFloat("property.roughness", 1.f);
  defMat->SetFloat("property.ao", 1.f);
  defMat->SetSampler2D("property.diffMap", ResourceManager::Get<Texture>("white"));
  defMat->SetSampler2D("property.normMap", ResourceManager::Get<Texture>("black"));
  defMat->SetSampler2D("property.RMAMap", ResourceManager::Get<Texture>("white"));
  Add<Material>(defMat, "default");

  Add<Model>(ModelGenerator::CreateCube(), "default");
  Add<Model>(ModelGenerator::CreateLineCube(), "debug line cube");
  Add<Model>(ModelGenerator::CreateCircle(0.5f, 32), "debug line circle");
  Add<Model>(ModelGenerator::CreateSquare(), "debug line square");
  Add<Model>(ModelGenerator::CreateLineSphere(16), "debug line sphere");
  Add<Model>(ModelGenerator::CreateSphere(0.5f, 16), "debug sphere");
  Add<Model>(ModelGenerator::CreateLine(), "debug line");
  Add<Model>(ModelGenerator::CreateLineTriangle(), "debug line triangle");
  Add<Model>(ModelGenerator::CreateLineCircleSphere(0.5f, 32), "debug line circle sphere");
  Add<Model>(ModelGenerator::CreateQuad(), "quad");
  Add<Model>(ModelGenerator::CreateCube(), "cube");
  Add<Model>(ModelGenerator::CreateSphere(0.1f, 16), "sphere");
  Add<Model>(ModelGenerator::CreateSphere(0.05f, 4), "light sphere");

  Add<SkinnedMesh>(ReadAssimpFile("../Common/models/bunny.obj"), "default");
}

void ResourceManager::Exit()
{
  LOG_TRACE("Resource", "Clearing Resource Manager...");

  delete Quad;

  for (IResourceMap* resourceMap : ResourceMaps)
  {
    resourceMap->Clear();
    delete resourceMap;
  }
  ResourceMaps.clear();
  NextResourceMapID = 0;
  EditorTypeID = 0;
}

//void ResourceManager::EditTexture(Texture* texture, const std::string& name)
//{
//  Editor::SelectMenu("Right Editor", "Resource Editor");
//
//  editName = name;
//  editTexture = texture;
//  editModel = nullptr;
//  editMaterial = nullptr;
//  editShader = nullptr;
//}
//
//void ResourceManager::EditModel(Model* mesh, const std::string& name)
//{
//  Editor::SelectMenu("Right Editor", "Resource Editor");
//
//  editName = name;
//  editTexture = nullptr;
//  editModel = mesh;
//  editMaterial = nullptr;
//  editShader = nullptr;
//}
//
//void ResourceManager::EditMaterial(Material* material, const std::string& name)
//{
//  Editor::SelectMenu("Right Editor", "Resource Editor");
//
//  editName = name;
//  editTexture = nullptr;
//  editModel = nullptr;
//  editMaterial = material;
//  editShader = nullptr;
//}
//
//void ResourceManager::EditShader(Shader* shader, const std::string& name)
//{
//  Editor::SelectMenu("Right Editor", "Resource Editor");
//
//  editName = name;
//  editTexture = nullptr;
//  editModel = nullptr;
//  editMaterial = nullptr;
//  editShader = shader;
//}

void ResourceManager::DrawEditorMenu(Rect<unsigned int> window)
{
  ResourceMaps[EditorTypeID]->DrawEditorMenu(window);
}

void ResourceManager::SetEditType(unsigned int typeId)
{
  EditorTypeID = typeId;
}

void ResourceManager::DrawDebugMenu(Rect<unsigned int> window)
{
  static bool showIcons = false;

  static unsigned int typeID = 0;
  static const char* typeName;
  for (auto type : TypeToID)
  {
    if (ImGui::Button(type.first))
    {
      typeID = type.second;
      typeName = type.first;
    }

    ImGui::SameLine();
  }
  if (ImGui::Button("Icon Atlas")) showIcons = true;
  ImGui::Separator();

  ImGui::BeginChild("Resource List", ImVec2(window.ScaleX() - 20, window.ScaleY() - 75), true);
  if (TypeToID.empty() == false)
  {
    ResourceMaps[typeID]->DrawDebugMenu(window);
  }
  ImGui::EndChild();

  if (showIcons)
  {
    ImGui::Begin("Icone List", &showIcons);
    for (auto it : ResourceMaps)
    {
      if (it->IsHavePreview())
      {
        ImGui::ImageButton(ImTextureID(it->GetPreviewTexture()->GetTexture()->GetID()), ImVec2(400, 400));
      }
    }
    ImGui::End();
  }
}

//void ResourceManager::DrawTextureSelector(Texture** outTexture, bool* windowFlag)
//{
//  ImGui::SetNextWindowSize(ImVec2(300, 300));
//  ImGui::Begin("Texture Selector");
//  auto it = textures.GetMap().begin();
//  int i = 0;
//  for (; it != textures.GetMap().end(); it++)
//  {
//    if (i % 3) ImGui::SameLine();
//    if (ImGui::ImageButton(ImTextureID(it->second->Data->GetID()), ImVec2(80.f, 80.f)))
//    {
//      *outTexture = it->second->Data;
//      *windowFlag = false;
//    }
//    
//    i++;
//  }
//  ImGui::End();
//}
//
//void ResourceManager::DrawMeshSelector(Model** outModel, bool* windowFlag)
//{
//  ImGui::SetNextWindowSize(ImVec2(300, 300));
//  ImGui::Begin("Mesh Selector");
//  auto it = meshes.GetMap().begin();
//  TextureAtlas* meshAtlas = meshes.GetPreviewTexture();
//  int i = 0;
//  for (; it != meshes.GetMap().end(); it++)
//  {
//    if (i % 3) ImGui::SameLine();
//
//    glm::vec4 tileuv = meshAtlas->GetTileUV(it->second->PreviewID);
//    ImVec2 topright(tileuv.x + tileuv.z, tileuv.y);
//    ImVec2 bottomleft(tileuv.x, tileuv.y - tileuv.w);
//
//    if (ImGui::ImageButton(ImTextureID(meshAtlas->GetTexture()->GetID()), ImVec2(80.f, 80.f), bottomleft, topright))
//    {
//      *outModel = it->second->Data;
//      *windowFlag = false;
//    }
//
//    i++;
//  }
//  ImGui::End();
//}
//
//void ResourceManager::DrawMaterialSelector(Material** outMaterial, bool* windowFlag)
//{
//  ImGui::SetNextWindowSize(ImVec2(300, 300));
//  ImGui::Begin("Material Selector");
//  auto it = materials.GetMap().begin();
//  int i = 0;
//  for (; it != materials.GetMap().end(); it++)
//  {
//    if (i % 3) ImGui::SameLine();
//    if (ImGui::Button(it->first.c_str()))
//    {
//      *outMaterial = it->second->Data;
//      *windowFlag = false;
//    }
//
//    i++;
//  }
//  ImGui::End();
//}