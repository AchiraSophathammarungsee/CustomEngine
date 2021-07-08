#include "ColliderSystem.h"
#include "Transform.h"
#include "Coordinator.h"
#include "Collider.h"
#include "Renderer.h"

void ColliderBoxSystem::Update()
{
  for (EntityID entity : Entities)
  {
    auto& transform = Coordinator::GetComponent<Transform>(entity);
    auto& collider = Coordinator::GetComponent<ColliderBox>(entity);

    collider.Center = transform.GetPosition();

    if(collider.ShowDebug) Renderer::DebugLineCube(Transform(collider.Center, glm::vec3(0.f, 1.f, 0.f), collider.Scale), glm::vec3(1.f, 1.f, 0.f));
  }
}

//bool ColliderBoxSystem::PointTest(const glm::vec3& point)
//{
//  for (EntityID entity : Entities)
//  {
//    auto& transform = Coordinator::GetComponent<Transform>(entity);
//    auto& collider = Coordinator::GetComponent<ColliderBox>(entity);
//
//    
//  }
//}

void ColliderSphereSystem::Update()
{
  for (EntityID entity : Entities)
  {
    auto& transform = Coordinator::GetComponent<Transform>(entity);
    auto& collider = Coordinator::GetComponent<ColliderSphere>(entity);

    collider.Center = transform.GetPosition();

    if (collider.ShowDebug) Renderer::DebugLineCircleSphere(Transform(collider.Center, glm::vec3(0.f, 1.f, 0.f), glm::vec3(collider.Radius * 2.f)), glm::vec3(1.f, 1.f, 0.f));
  }
}