#include "ColliderManager.h"
#include "ColliderSystem.h"
#include "Coordinator.h"
#include "Transform.h"
#include "Collider.h"
#include "CollisionSolver.h"

std::shared_ptr<ColliderBoxSystem> ColliderManager::BoxSystem;
std::shared_ptr<ColliderSphereSystem> ColliderManager::SphereSystem;

void ColliderManager::Init(std::shared_ptr<ColliderBoxSystem> boxsys, std::shared_ptr<ColliderSphereSystem> spheresys)
{
  BoxSystem = boxsys;
  SphereSystem = spheresys;
}

void ColliderManager::Update()
{
  /*std::vector<EntityID> &boxEntities = BoxSystem->GetEntities();
  std::vector<EntityID> &sphereEntities = SphereSystem->GetEntities();

  int totalEntities = boxEntities.size() + sphereEntities.size();*/

  BoxSystem->Update();
  SphereSystem->Update();
}

CollisionInfo ColliderManager::PointTest(const glm::vec3& point)
{
  std::vector<EntityID>& boxEntities = BoxSystem->GetEntities();
  std::vector<EntityID>& sphereEntities = SphereSystem->GetEntities();

  for (EntityID ent : boxEntities)
  {
    auto& box = Coordinator::GetComponent<ColliderBox>(ent);

    BoundingVolume bv = BoundingVolume(box.Center, box.Center - box.Scale * 0.5f, box.Center + box.Scale * 0.5f);
    if (CollisionSolver::PointBoxTest(bv, point)) return CollisionInfo(true, point, glm::normalize(point - box.Center), &box);
  }

  for (EntityID ent : sphereEntities)
  {
    auto& sphere = Coordinator::GetComponent<ColliderSphere>(ent);

    if (CollisionSolver::PointSphereTest(BoundingVolume(sphere.Center, sphere.Radius), point)) return CollisionInfo(true, point, glm::normalize(point - sphere.Center), &sphere);
  }

  return CollisionInfo();
}

CollisionInfo ColliderManager::RayTest(const Ray& ray, float length)
{
  std::vector<EntityID>& boxEntities = BoxSystem->GetEntities();
  std::vector<EntityID>& sphereEntities = SphereSystem->GetEntities();

  for (EntityID ent : boxEntities)
  {
    auto& box = Coordinator::GetComponent<ColliderBox>(ent);

    BoundingVolume bv = BoundingVolume(box.Center, box.Center - box.Scale * 0.5f, box.Center + box.Scale * 0.5f);
    if (CollisionSolver::RayBoxTest(bv, ray, length)) return CollisionInfo(true, ray.Origin(), ray.Dir(), &box);
  }

  for (EntityID ent : sphereEntities)
  {
    auto& sphere = Coordinator::GetComponent<ColliderSphere>(ent);

    if (CollisionSolver::RaySphereTest(BoundingVolume(sphere.Center, sphere.Radius), ray, length)) return CollisionInfo(true, ray.Origin(), ray.Dir(), &sphere);
  }

  return CollisionInfo();
}