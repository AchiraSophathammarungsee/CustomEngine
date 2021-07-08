#include "PostProcess.h"
#include "Material.h"
#include "Framebuffer.h"
#include "shader.hpp"
#include "Model.h"
#include <imgui.h>
#include "ResourceManager.h"

PostProcess::PostProcess(const char* name, Material* _material, Framebuffer* _input, Framebuffer* _output) : name(name), material(_material), input(_input), output(_output)
{

}

void PostProcess::Draw(Model* quad)
{
  output->BeginCapture();

  input->BindAllTextures(0);
  material->UpdateUniform();
  
  quad->Draw();

  output->EndCapture();
}

void PostProcess::DrawDebugMenu()
{
  ImGui::Text("Name: %s", name.c_str());

  static bool changeMat = false;
  if (ImGui::Button("Change Material"))
  {
    changeMat = true;
  }
  if (changeMat)
  {
    ResourceManager::DrawResourceSelector<Material>(&material, &changeMat);
  }
}