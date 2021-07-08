#include "CollisionSolver.h"
#include "Coordinator.h"
#include "Renderer.h"
#include "MathUtil.h"

float squaredDist(const glm::vec3& a, const glm::vec3& b)
{
  return MathUtil::SqrDist(a - b);
}

bool CollisionSolver::PointBoxTest(const BoundingVolume& box, const glm::vec3& p)
{
  //BoundingBox BB = box.BoundBox();

  return (p.x >= box.min.x && p.x <= box.max.x) &&
         (p.y >= box.min.y && p.y <= box.max.y) &&
         (p.z >= box.min.z && p.z <= box.max.z);
}

bool CollisionSolver::PointSphereTest(const BoundingVolume& sphere, const glm::vec3 & p)
{
  float distsqr = squaredDist(sphere.center, p);
  return distsqr < (sphere.radius * sphere.radius);
}

bool CollisionSolver::RayBoxTest(const BoundingVolume& box, const Ray& ray, float length)
{
  //const BoundingBox& BB = box.BB;

  float tmin = 0.f, tmax = 0.f, tymin = 0.f, tymax = 0.f, tzmin = 0.f, tzmax = 0.f;

  /*tmin = (bounds[ray.Sign()[0]].x - ray.Origin().x) * ray.InvDir().x;
  tmax = (bounds[1 - ray.Sign()[0]].x - ray.Origin().x) * ray.InvDir().x;
  tymin = (bounds[ray.Sign()[1]].y - ray.Origin().y) * ray.InvDir().y;
  tymax = (bounds[1 - ray.Sign()[1]].y - ray.Origin().y) * ray.InvDir().y;*/

  if ((tmin > tymax) || (tymin > tmax))
    return false;
  if (tymin > tmin)
    tmin = tymin;
  if (tymax < tmax)
    tmax = tymax;

  //tzmin = (bounds[ray.Sign()[2]].z - ray.Origin().z) * ray.InvDir().z;
  //tzmax = (bounds[1 - ray.Sign()[2]].z - ray.Origin().z) * ray.InvDir().z;

  if ((tmin > tzmax) || (tzmin > tmax))
    return false;
  if (tzmin > tmin)
    tmin = tzmin;
  if (tzmax < tmax)
    tmax = tzmax;

  return true;
}

bool CollisionSolver::RaySphereTest(const BoundingVolume& sphere, const Ray& ray, float length)
{
  glm::vec3 toOrigin = ray.Origin() - sphere.center;

  float a = 1.f;
  float b = 2.f * glm::dot(toOrigin, ray.Dir());
  float c = glm::dot(toOrigin, toOrigin) - sphere.radius * sphere.radius;

  float t0, t1;
  if (MathUtil::SolveQuadratic(a, b, c, t0, t1) == false) return false;

  if (t0 < 0) {
    t0 = t1; // if t0 is negative, let's use t1 instead
    if (t0 < 0) return false; // both t0 and t1 are negative
  }

  float t = t0;

  if (t > length) return false;
  
  return true;
}

bool CollisionSolver::BoxBoxTest(const BoundingVolume& a, const BoundingVolume& b)
{
  return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
         (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
         (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

bool CollisionSolver::BoxSphereTest(const BoundingVolume& box, const BoundingVolume& sphere)
{
  // get point of box closest to sphere center
  glm::vec3 closestPoint(0.f);

  closestPoint.x = glm::max(box.min.x, glm::min(sphere.center.x, box.max.x));
  closestPoint.y = glm::max(box.min.y, glm::min(sphere.center.y, box.max.y));
  closestPoint.z = glm::max(box.min.z, glm::min(sphere.center.z, box.max.z));

  float distsqr = squaredDist(closestPoint, sphere.center);
  return distsqr < (sphere.radius * sphere.radius);
}

bool CollisionSolver::SphereSphereTest(const BoundingVolume& a, const BoundingVolume& b)
{
  float distsqr = squaredDist(a.center, b.center);
  return distsqr < (a.radius + b.radius) * (a.radius + b.radius);
}

/*
void CollisionSolver::Update()
{
  for (int i = 0; i < Entities.size(); i++)
  {
    EntityID a = Entities[i];
    auto& aCollider = Coordinator::GetComponent<Collider>(a);
    auto& aTransform = Coordinator::GetComponent<Transform>(a);

    DrawDebugCollider(aTransform.GetPosition(), aCollider);

    for (int j = i + 1; i < Entities.size(); j++)
    {
      EntityID b = Entities[j];

      auto& bCollider = Coordinator::GetComponent<Collider>(b);
      auto& bTransform = Coordinator::GetComponent<Transform>(b);

      if (Intersect(aTransform.GetPosition(), aCollider, bTransform.GetPosition(), bCollider))
      {
        LOG_TRACE_S("CollisionSolver", "Collider from entity {} has collided with {}", a, b);
      }
    }
  }

  //std::memset(CheckedCollider, false, sizeof(CheckedCollider));
}

bool CollisionSolver::Intersect(const glm::vec3& aPos, const Collider& a, const glm::vec3& bPos, const Collider& b) const
{
  glm::vec3 aCenter = aPos + a.Offset;
  glm::vec3 bCenter = bPos + b.Offset;

  float aMinX = aCenter.x - a.Scale.x / 2.f;
  float aMaxX = aCenter.x + a.Scale.x / 2.f;
  float aMinY = aCenter.y - a.Scale.y / 2.f;
  float aMaxY = aCenter.y + a.Scale.y / 2.f;
  float aMinZ = aCenter.z - a.Scale.z / 2.f;
  float aMaxZ = aCenter.z + a.Scale.z / 2.f;

  float bMinX = bCenter.x - b.Scale.x / 2.f;
  float bMaxX = bCenter.x + b.Scale.x / 2.f;
  float bMinY = bCenter.y - b.Scale.y / 2.f;
  float bMaxY = bCenter.y + b.Scale.y / 2.f;
  float bMinZ = bCenter.z - b.Scale.z / 2.f;
  float bMaxZ = bCenter.z + b.Scale.z / 2.f;

  return (aMinX <= bMaxX && aMaxX >= bMinX) &&
         (aMinY <= bMaxY && aMaxY >= bMinY) &&
         (aMinZ <= bMaxZ && aMaxZ >= bMinZ);
}*/

//void CollisionSolver::Solve(Transform& aTransform, const Collider& aCollider, glm::vec3& bTransform, const Collider& bCollider)
//{
//  if (aCollider.Ghost || bCollider.Ghost) return;
//
//
//}
//
//void CollisionSolver::DrawDebugCollider(const glm::vec3 &pos, const Collider& col)
//{
//  //Renderer::DebugLineCube(Transform(pos + col.Offset, glm::vec3(0.f, 1.f, 0.f), col.Scale), glm::vec3(1.f, 1.f, 0.f));
//}