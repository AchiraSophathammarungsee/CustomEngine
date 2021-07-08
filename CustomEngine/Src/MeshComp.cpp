#include "MeshComp.h"
#include "ResourceManager.h"
#include "Model.h"

MeshComp::MeshComp(Model* model, Material* _material) : mesh(model), material(_material), hide(false)
{
  if (mesh == nullptr)
  {
    mesh = ResourceManager::Get<Model>("default");
  }
  if (material == nullptr)
  {
    material = ResourceManager::Get<Material>("default");
  }
}

MeshComp::~MeshComp()
{

}

void MeshComp::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("Mesh Renderer"))
  {
    ImGui::Separator();
    ImGui::Indent(10.f);
    
    static bool selectingMesh = false;
    if (ImGui::Button("Select Mesh"))
    {
      selectingMesh = true;
    }
    if (selectingMesh)
    {
      ResourceManager::DrawResourceSelector<Model>(&mesh, &selectingMesh);
    }

    mesh->DrawDebugMenu();
    ImGui::Text("Material: %s", material->GetShader()->GetName().c_str());
    static bool selectMat = false;
    if (ImGui::Button("Select Material"))
    {
      selectMat = true;
    }
    if (selectMat)
    {
      ResourceManager::DrawResourceSelector<Material>(&material, &selectMat);
    }

    boundingVolume.DrawDebugMenu();

    ImGui::Unindent(10.f);
    ImGui::Separator();
  }
}