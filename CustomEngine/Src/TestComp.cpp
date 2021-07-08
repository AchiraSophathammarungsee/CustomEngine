#include "TestComp.h"

#include "EngineCore.h"

void TestComp::DrawDebugMenu()
{
  if(ImGui::CollapsingHeader("Test Comp"))
  {
    ImGui::InputFloat("Rotate speed", &rotateSpeed);
  }
}