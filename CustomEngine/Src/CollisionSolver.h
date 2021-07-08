
#ifndef COLLISIONSOLVER_H
#define COLLISIONSOLVER_H

//#include "Collider.h"
#include "Transform.h"
#include "Ray.h"
#include "BoundingVolume.h"

class CollisionSolver
{
public:
  //void Update();
  //bool Intersect(const glm::vec3& aPos, const Collider& a, const glm::vec3& bPos, const Collider& b) const;

  static bool PointBoxTest(const BoundingVolume& box, const glm::vec3& p);
  static bool PointSphereTest(const BoundingVolume& sphere, const glm::vec3& p);
  static bool RayBoxTest(const BoundingVolume& box, const Ray& ray, float length);
  static bool RaySphereTest(const BoundingVolume& sphere, const Ray& ray, float length);
  static bool BoxBoxTest(const BoundingVolume& a, const BoundingVolume& b);
  static bool BoxSphereTest(const BoundingVolume& box, const BoundingVolume& sphere);
  static bool SphereSphereTest(const BoundingVolume& a, const BoundingVolume& b);

private:
  CollisionSolver() {};
  ~CollisionSolver() {};

  //void Solve(Transform& aTransform, const Collider& aCollider, glm::vec3& bTransform, const Collider& bCollider);

  //void DrawDebugCollider(const glm::vec3 &pos, const Collider& col);

  //bool CheckedCollider[MAX_ENTITIES][MAX_ENTITIES];
};

#endif