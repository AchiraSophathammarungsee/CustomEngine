#include "Gravity.h"
#include "Coordinator.h"
#include "Movement.h"
#include "Mass.h"

void Gravity::Update()
{
  for (EntityID entity : Entities)
  {
    auto& movement = Coordinator::GetComponent<Movement>(entity);
    auto& mass = Coordinator::GetComponent<Mass>(entity);

    movement.Velocity.y -= GravityConstant;
  }
}

void Gravity::DrawDebugMenu()
{
  ImGui::InputFloat("Gravity constant", &GravityConstant);
}