#include "IKSystem.h"
#include "Coordinator.h"
#include "Time.h"
#include "SkinnedMesh.h"
#include "Renderer.h"
#include "Quaternion.h"
#include <glm/glm.hpp>

void IKSystem::Init()
{
  for (EntityID entity : Entities)
  {
    auto& mesh = Coordinator::GetComponent<SkinnedMeshComp>(entity);
    auto& ik = Coordinator::GetComponent<IKAnimation>(entity);

    ik.Mesh = &mesh;
    mesh.mesh->SetUseIK(true);
  }
}

void IKSystem::Update()
{
  for (EntityID entity : Entities)
  {
    auto& transform = Coordinator::GetComponent<Transform>(entity);
    auto& mesh = Coordinator::GetComponent<SkinnedMeshComp>(entity);
    auto& ik = Coordinator::GetComponent<IKAnimation>(entity);

    if (ik.Playing)
    {
      CCD(transform, mesh, ik);

      ik.AnimTimer += Time::DT();
      if (ik.AnimTimer > ik.AnimDuration)
      {
        ik.Stop();
      }
    }

    Renderer::DebugLineSphere(Transform(ik.Manipulator, glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.2f)), glm::vec3(1.f, 1.f, 0.f));
  }
}

int GetChainBoneID(int effectID, int length, std::vector<Bone> &bones)
{
  int id = effectID;
  for (int i = 0; i < length; i++)
  {
    id = bones[id].Parent;
  }
  return id;
}

void UpdateBoneMatrix(std::vector<Bone>& bones, Bone* currBone)
{
  if (currBone->Parent == -1)
  {
    currBone->TransformMatrix = currBone->BindposeMatrix * currBone->IKTransform;
  }
  else
  {
    currBone->TransformMatrix = bones[currBone->Parent].TransformMatrix * currBone->BindposeMatrix * currBone->IKTransform;
  }

  currBone->FinalMatrix = currBone->TransformMatrix * currBone->OffsetMatrix;

  for (unsigned int child : currBone->Childrens)
  {
    UpdateBoneMatrix(bones, &bones[child]);
  }
}

void IKSystem::CCD(Transform &transform, SkinnedMeshComp& mesh, IKAnimation& ik)
{
  std::vector<Bone>& bones = mesh.mesh->GetBones();

  glm::vec3 goal = glm::inverse(transform.GetMatrix()) * glm::vec4(ik.Manipulator, 1.f);
  glm::vec3 frameGoal = (1.f - (ik.AnimTimer / ik.AnimDuration)) * ik.InitEndEffectPos + (ik.AnimTimer / ik.AnimDuration) * goal;
  glm::vec3 currEndEffector = bones[ik.EndEffectorBone].TransformMatrix * glm::vec4(0.f, 0.f, 0.f, 1.f);

  // loop until end effector is within frameGoal's tolerance or no longer making progress
  float prevDist = 1000.f;
  float dist = glm::length(frameGoal - currEndEffector);
  float progressDist = dist - prevDist;
  progressDist = progressDist > 0.f ? progressDist : -progressDist;
  while ((dist > ik.Tolerance) && (progressDist > ik.MinDistProgress))
  {
    for (int i = 1; i <= ik.ChainLength; i++)
    {
      int boneID = GetChainBoneID(ik.EndEffectorBone, i, bones);

      glm::vec3 originRot = bones[boneID].TransformMatrix * glm::vec4(0.f, 0.f, 0.f, 1.f);
      currEndEffector = bones[ik.EndEffectorBone].TransformMatrix * glm::vec4(0.f, 0.f, 0.f, 1.f);

      glm::vec3 b = glm::normalize(currEndEffector - originRot);
      glm::vec3 c = glm::normalize(frameGoal - originRot);

      float angle = glm::acos(glm::dot(b, c));
      angle = glm::degrees(angle);
      glm::vec3 rotAxis = glm::cross(b, c);

      rotAxis = glm::inverse(bones[boneID].TransformMatrix) * glm::vec4(rotAxis, 0.f);
      glm::mat4 rotMat = Quaternion::AxisAngle(rotAxis, angle).GetMatrix();
      bones[boneID].IKTransform = bones[boneID].IKTransform * rotMat;

      // run forward kinematic algorithm to update all bones.
      UpdateBoneMatrix(bones, &bones[0]);
    }

    prevDist = dist;
    dist = glm::length(frameGoal - currEndEffector);
    progressDist = dist - prevDist;
    progressDist = progressDist > 0.f ? progressDist : -progressDist;
  }
}