#include "Frustum.h"

void Frustum::UpdateProjection(float angle, float ratio, float nearD, float farD)
{
  // store the information
  this->ratio = ratio;
  this->angle = angle;
  this->nearD = nearD;
  this->farD = farD;

  // compute width and height of the near and far plane sections
  tang = (float)tan(glm::radians(angle) * 0.5);
  nh = nearD * tang;
  nw = nh * ratio;
  fh = farD * tang;
  fw = fh * ratio;
}

void Frustum::UpdateOrtho(float width, float height, float nearD, float farD)
{
  // store the information
  this->ratio = width / height;
  this->angle = 0.f;
  this->nearD = nearD;
  this->farD = farD;

  nh = height;
  nw = nh * ratio;
  fh = height;
  fw = fh * ratio;
}

void Frustum::UpdateLookAt(const glm::vec3& camPos, const glm::vec3& viewDir, const glm::vec3& upVec)
{
  glm::vec3 rightVec = glm::normalize(glm::cross(upVec, viewDir));

  glm::vec3 realUp = glm::cross(viewDir, rightVec);

  // compute the centers of the near and far planes
  nc = camPos + viewDir * nearD;
  fc = camPos + viewDir * farD;

  // compute the 4 corners of the frustum on the near plane
  ntl = nc + realUp * nh - rightVec * nw;
  ntr = nc + realUp * nh + rightVec * nw;
  nbl = nc - realUp * nh - rightVec * nw;
  nbr = nc - realUp * nh + rightVec * nw;

  // compute the 4 corners of the frustum on the far plane
  ftl = fc + realUp * fh - rightVec * fw;
  ftr = fc + realUp * fh + rightVec * fw;
  fbl = fc - realUp * fh - rightVec * fw;
  fbr = fc - realUp * fh + rightVec * fw;

  // compute the six planes
  // the function set3Points assumes that the points
  // are given in counter clockwise order
  planes[PLANE_TOP].From3Points(ntr, ntl, ftl);
  planes[PLANE_BOTTOM].From3Points(nbl, nbr, fbr);
  planes[PLANE_LEFT].From3Points(ntl, nbl, fbl);
  planes[PLANE_RIGHT].From3Points(nbr, ntr, fbr);
  planes[PLANE_NEAR].From3Points(ntl, ntr, nbr);
  planes[PLANE_FAR].From3Points(ftr, ftl, fbl);

  /*planes[PLANE_NEAR].FromPointNormal(-viewDir, nc);
  planes[PLANE_FAR].FromPointNormal(viewDir, fc);

  glm::vec3 aux, normal;

  aux = glm::normalize((nc + realUp * nh) - camPos);
  normal = glm::cross(aux, rightVec);
  planes[PLANE_TOP].FromPointNormal(normal, nc + realUp * nh);

  aux = glm::normalize((nc - realUp * nh) - camPos);
  normal = glm::cross(rightVec, aux);
  planes[PLANE_BOTTOM].FromPointNormal(normal, nc - realUp * nh);

  aux = glm::normalize((nc - rightVec * nw) - camPos);
  normal = glm::cross(aux, realUp);
  planes[PLANE_LEFT].FromPointNormal(normal, nc - rightVec * nw);

  aux = glm::normalize((nc + rightVec * nw) - camPos);
  normal = glm::cross(realUp, aux);
  planes[PLANE_RIGHT].FromPointNormal(normal, nc + rightVec * nw);*/
}

bool Frustum::FastCullSphere(const glm::vec3& center, float radius) const
{
  for (int i = 0; i < 6; i++)
  {
    float dist = planes[i].Dist(center);

    if (dist > radius)
    {
      return false;
    }
  }
  return true;
}
bool Frustum::FastCullAABB(const glm::vec3& max, const glm::vec3& min) const
{
  for (int i = 0; i < 6; i++)
  {
    glm::vec3 positiveP = max;

    if (planes[i].normal.x > 0.f)
    {
      positiveP.x = min.x;
    }

    if (planes[i].normal.z > 0.f)
    {
      positiveP.z = min.z;
    }

    if (planes[i].normal.y > 0.f)
    {
      positiveP.y = min.y;
    }

    if (planes[i].Dist(positiveP) > 0.f)
    {
      return false;
    }
  }
  return true;
}

bool Frustum::CullSphere(const glm::vec3& center, float radius, Bitmask& insections) const
{
  bool inside = true;
  for (int i = 0; i < 6; i++)
  {
    float dist = planes[i].Dist(center);

    if (dist > radius)
    {
      inside = false;
    }
    else if (dist < -radius)
    {
      insections.SetBit(i, true);
    }
  }
  return true;
}

bool Frustum::CullAABB(const glm::vec3& max, const glm::vec3& min, Bitmask& insections) const
{
  bool inside = true;
  for (int i = 0; i < 6; i++)
  {
    glm::vec3 positiveP = max;
    glm::vec3 negP = min;

    if (planes[i].normal.x > 0.f)
    {
      positiveP.x = min.x;
      negP.x = max.x;
    }

    if (planes[i].normal.z > 0.f)
    {
      positiveP.z = min.z;
      negP.z = max.z;
    }

    if (planes[i].normal.y > 0.f)
    {
      positiveP.y = min.y;
      negP.y = max.y;
    }

    if (planes[i].Dist(positiveP) > 0.f)
    {
      inside = false;
    }
    else if(planes[i].Dist(negP) > 0.f)
      insections.SetBit(i, true);
  }
  return inside;
}