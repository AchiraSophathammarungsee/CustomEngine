#include "IKAnimation.h"
#include <imgui.h>
#include "SkinnedMeshComp.h"
#include "SkinnedMesh.h"
#include "EngineCore.h"

void IKAnimation::Play()
{
  // check if end effector bone id is valid
  if (EndEffectorBone == -1)
  {
    LOG_ERROR("IKAnimation", "End effector is -1");
    return;
  }
  if (ChainLength < 1)
  {
    LOG_ERROR("IKAnimation", "Chaing length is less than 1");
    return;
  }

  Playing = true;
  AnimTimer = 0.f;

  Mesh->mesh->SetUseAnimation(true);
  
  std::vector<Bone> &bones = Mesh->mesh->GetBones();
  Mesh->mesh->ForwardKinematic(&bones[0]);
  InitEndEffectPos = bones[EndEffectorBone].TransformMatrix * glm::vec4(0.f, 0.f, 0.f, 1.f);

  for (Bone& b : bones)
  {
    b.IKTransform = glm::mat4(1.f);
  }
}

void IKAnimation::Stop()
{
  Playing = false;
}

void IKAnimation::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("IK Animation"))
  {
    ImGui::Text("Time: %f", AnimTimer);
    ImGui::InputFloat("Influence", &Influence);
    ImGui::InputFloat3("Manipulator", &Manipulator[0]);
    ImGui::InputFloat("Animation Duration", &AnimDuration);
    ImGui::InputInt("End Effector", &EndEffectorBone);
    ImGui::InputInt("Chain Length", &ChainLength);
    ImGui::InputFloat("Tolerance", &Tolerance);
    ImGui::InputFloat("MinDistProgress", &MinDistProgress);

    if (Playing)
    {
      if (ImGui::Button("Stop"))
      {
        Stop();
      }
    }
    else
    {
      if (ImGui::Button("Play"))
      {
        Play();
      }
    }
  }
}