#include "VQS.h"
#include "EngineCore.h"

VQS::VQS(const glm::vec3& position, const glm::vec3& scale, const Quaternion& rotation) : Position(position), Scale(scale), Rotation(rotation), NeedUpdate(true), Matrix(1.f)
{
  UpdateMatrix();
}

void VQS::UpdateMatrix()
{
  if (NeedUpdate)
  {
    glm::mat4 posMat(1.f);
    glm::mat4 scaleMat(1.f);

    scaleMat[0][0] = Scale.x;
    scaleMat[1][1] = Scale.y;
    scaleMat[2][2] = Scale.z;

    posMat[3][0] = Position.x;
    posMat[3][1] = Position.y;
    posMat[3][2] = Position.z;

    Matrix = posMat * Rotation.GetMatrix() * scaleMat;

    NeedUpdate = false;
  }
}

const glm::mat4& VQS::GetMatrix() const
{ 
  ASSERT(NeedUpdate == false, "[VQS]: tried to access outdated matrix!");

  return Matrix;
}

glm::vec4 VQS::operator*(const glm::vec4& vector) const
{
  return GetMatrix() * vector;
}

VQS VQS::operator*(const VQS& other) const
{
  glm::vec4 newPos = GetMatrix() * glm::vec4(other.Position, 1.0f);

  return VQS(glm::vec3(newPos.x, newPos.y, newPos.z), other.Scale * Scale, Rotation * other.Rotation);
}

VQS VQS::Inverse() const
{
  Quaternion invertQ = Rotation.Inverse();
  //glm::vec3 invertS = ;

  //return VQS(invertQ * -1 * (invertS * Position) * Rotation, invertS, invertQ);
  return VQS();
}

VQS VQS::Lerp(const VQS& a, const VQS& b, float ratio)
{
  glm::vec3 position = (1.0f - ratio) * a.GetPosition() + (ratio) * b.GetPosition();
  glm::vec3 scale = (1.0f - ratio) * a.GetScale() + (ratio) * b.GetScale();
  Quaternion rotation = Quaternion::Slerp(a.GetRotation(), b.GetRotation(), ratio);

  return VQS(position, scale, rotation);
}
