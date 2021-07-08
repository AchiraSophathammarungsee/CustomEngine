#include "Movement.h"
#include "EngineCore.h"

void Movement::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("Movement"))
  {
    ImGui::Separator();
    ImGui::Indent(10.f);

    ImGui::InputFloat3("Velocity", &Velocity[0]);
    ImGui::InputFloat3("Acceleration", &Acceleration[0]);

    ImGui::Unindent(10.f);
    ImGui::Separator();
  }
}
