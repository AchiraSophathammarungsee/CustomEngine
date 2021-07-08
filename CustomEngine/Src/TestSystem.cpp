#include "TestSystem.h"
#include "Transform.h"
#include "TestComp.h"
#include "Time.h"
#include "Coordinator.h"

void TestSystem::Update()
{
  for (auto const& entity : Entities)
  {
    auto& transform = Coordinator::GetComponent<Transform>(entity);
    auto const& rotor = Coordinator::GetComponent<TestComp>(entity);

    transform.SetRotationAngle(transform.GetRotationAngle() + rotor.rotateSpeed * Time::DT());
  }
}