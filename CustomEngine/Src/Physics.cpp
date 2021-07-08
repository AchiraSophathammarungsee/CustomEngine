#include "Physics.h"
#include "Coordinator.h"
#include "Movement.h"
#include "Transform.h"

void Physics::Update()
{
  for (EntityID entity : Entities)
  {
    auto& movement = Coordinator::GetComponent<Movement>(entity);
    auto& transform = Coordinator::GetComponent<Transform>(entity);

    transform.SetPosition(transform.GetPosition() + movement.Velocity * Time::DT());
  }
}