#include "SkinnedMeshComp.h"
#include "ResourceManager.h"
#include "SkinnedMesh.h"
#include "Material.h"

SkinnedMeshComp::SkinnedMeshComp(SkinnedMesh* model, Material* _material) : mesh(model), material(_material), hide(false)
{
  if (mesh == nullptr)
  {
    mesh = ResourceManager::Get<SkinnedMesh>("default");
  }
  if (material == nullptr)
  {
    material = ResourceManager::Get<Material>("default");
  }
}

SkinnedMeshComp::~SkinnedMeshComp()
{

}

void SkinnedMeshComp::DrawDebugMenu()
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
      ResourceManager::DrawResourceSelector<SkinnedMesh>(&mesh, &selectingMesh);
    }

    mesh->DrawDebugMenu();
    if (ImGui::CollapsingHeader("Material Resource"))
    {
      ImGui::Separator();
      material->DrawDebugMenu();
      ImGui::Separator();
    }

    ImGui::Unindent(10.f);
    ImGui::Separator();
  }
}