#include "Mass.h"
#include "EngineCore.h"

void Mass::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("Mass"))
  {
    ImGui::InputFloat("Current Mass", &CurrentMass);
    ImGui::Checkbox("Have Gravity FIeld", &HaveGravityField);
  }
}