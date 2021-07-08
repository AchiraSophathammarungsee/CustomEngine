
#include "RenderObject.h"
#include "shader.hpp"
#include "Material.h"

void RenderObject::Draw(Shader* shader)
{
  transform.UpdateMatrix();
  shader->setMat4("modelMatrix", transform.GetMatrix());

  if (boneArray != nullptr)
  {
    shader->setMat4Array("bones[0]", *boneArray);
  }

  glBindVertexArray(VAO);
  glDrawElements(primitiveMode, IndiceCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void RenderObject::UpdateMaterialUniforms()
{
  material->UpdateUniform();
}

//void RenderObject::DrawDebugMenu()
//{
//  ImGui::Text("ID: %d", id);
//  ImGui::Checkbox("Hide", &hide);
//  transform.DrawDebugMenu();
//  //ImGui::Image(ImTextureID(model->GetPreviewTexture()), ImVec2(80, 80));
//  if (ImGui::Button("Edit Mesh"))
//    ResourceManager::SetEdit<Model>(model, "Render object's mesh");
//  ImGui::SameLine();
//  static bool selectingMesh = false;
//  if (ImGui::Button("Change Mesh"))
//    selectingMesh = true;
//  if (selectingMesh)
//    ResourceManager::DrawResourceSelector<Model>(&model, &selectingMesh);
//
//  ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Edit material and textures here");
//  static bool editMaterial = false;
//  if (ImGui::Button("Edit Material"))
//    ResourceManager::SetEdit<Material>(material, "Render object's material");
//  ImGui::SameLine();
//  static bool selectingMaterial = false;
//  if (ImGui::Button("Change Material"))
//    selectingMaterial = true;
//  if (selectingMaterial)
//    ResourceManager::DrawResourceSelector<Material>(&material, &selectingMaterial);
//
//  BV->DrawDebugMenu();
//}