
#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>

class Ray
{
public:
  Ray(const glm::vec3& origin, const glm::vec3& direction) : m_origin(origin), m_direction(direction)
  {
    Update();
  };

  void Update()
  {
    m_direction = glm::normalize(m_direction);
    m_invDir = 1.f / m_direction;
    m_sign[0] = (m_invDir.x < 0);
    m_sign[1] = (m_invDir.y < 0);
    m_sign[2] = (m_invDir.z < 0);
  }

  const glm::vec3 &Origin() const { return m_origin; };
  const glm::vec3 &Dir() const { return m_direction; };
  const glm::vec3 &InvDir() const { return m_invDir; };

  void SetOrigin(const glm::vec3& vec) { m_origin = vec; };
  void SetDirection(const glm::vec3& vec) { m_direction = vec; Update(); };

  const int* Sign() const { return m_sign; };

private:
  glm::vec3 m_origin;
  glm::vec3 m_direction;
  glm::vec3 m_invDir;
  int m_sign[3];
};

class RayHit
{
public:

private:

};

#endif