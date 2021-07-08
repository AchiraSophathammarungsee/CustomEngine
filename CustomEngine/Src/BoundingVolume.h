#ifndef BOUNDINGVOLUME_H
#define BOUNDINGVOLUME_H

#include <glm/glm.hpp>
#include "Transform.h"
#include "Vertex.h"
class RenderObject;
class Shader;

enum BoundingVolumeTypes { BVT_AABB, BVT_Sphere, BVT_COUNT };

class BoundingVolume
{
public:
  BoundingVolume(BoundingVolumeTypes type = BVT_Sphere) : type(type), radius(1.f), center()
  { 
    //UpdateVolume(1.f);
  };
  BoundingVolume(const glm::vec3& center, float radius) : type(BVT_Sphere), radius(radius), center(center) {};
  BoundingVolume(const glm::vec3& center, const glm::vec3 &min, const glm::vec3& max) : type(BVT_AABB), min(min), max(max), radius(1.f), center(center) {};

  ~BoundingVolume() {};

  void UpdateVolume(const glm::vec3 &pos, const float radius);
  void UpdateVolume(const glm::vec3 &pos, const std::vector<glm::vec3> &vertices);
  void Draw();

  BoundingVolumeTypes GetType() const { return type; };
  void ChangeSphere(float _radius) { type = BVT_Sphere; UpdateVolume(center, _radius); };
  void ChangeAABB(const std::vector<glm::vec3>& vertices) { type = BVT_AABB; UpdateVolume(center, vertices); };

  void ComputeAABB(std::vector<Vertex>& vertices);
  void ComputeCentroid(std::vector<Vertex>& vertices);

  bool DrawDebugMenu();

  glm::vec3 center;
  glm::vec3 min, max;
  float radius;
protected:

  void ComputeAABB();
  
  void ComputeCentroid();
  
  void ComputeRitter();
  void ComputeLarsson();
  void ComputeSpherePCA();
  void ComputeEllipsoidPCA();
  void ComputeOBBPCA();
  void ComputeCovarianceMatrix(std::vector<Vertex>& vertices, glm::mat3& outMatrix);

  BoundingVolumeTypes type;
};
#endif