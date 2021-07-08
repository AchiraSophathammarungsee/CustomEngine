#include "Skybox.h"
#include "ResourceManager.h"
#include "Model.h"
#include "Material.h"

#include <imgui.h>

Skybox::Skybox(Material* mat) : m_material(mat), m_cube(nullptr)
{
  m_cube = ResourceManager::Get<Model>("cube");
}

CubeMap* Skybox::GetCubeMap()
{
  return m_material->GetSamplerCubeTex("property.skyTex");
}

void Skybox::Draw()
{
  m_material->UpdateUniform();

  m_cube->Draw();
}

void Skybox::DrawDebugMenu()
{
  if (m_material)
    ImGui::Text("Current material: %s", m_material->GetShader()->GetName().c_str());
  else
    ImGui::Text("Current material: null");

  static bool selectMat = false;
  if (ImGui::Button("Select material"))
  {
    selectMat = true;
  }
  if (selectMat)
  {
    ResourceManager::DrawResourceSelector<Material>(&m_material, &selectMat);
  }
}