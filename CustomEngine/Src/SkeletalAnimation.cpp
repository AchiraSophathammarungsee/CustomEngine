#include "SkeletalAnimation.h"
#include <imgui.h>
#include "SkinnedMesh.h"
#include "ResourceManager.h"

VQS BoneAnimation::Lerp(double time)
{
  UpdateCurrFrame(time);

  if (MaxFrame())
  {
    return Keyframes.back().value;
  }
  else
  {
    float ratio = time - Keyframes[CurrFrame].time;
    ratio /= Keyframes[CurrFrame + 1].time - Keyframes[CurrFrame].time;
    return VQS::Lerp(Keyframes[CurrFrame].value, Keyframes[CurrFrame + 1].value, ratio);
  }
}

void BoneAnimation::UpdateCurrFrame(double time)
{
  for (unsigned int i = 0; i < Keyframes.size(); i++)
  {
    if (i == Keyframes.size() - 1)
    {
      CurrFrame = Keyframes.size() - 1;
      break;
    }

    if (time >= Keyframes[i].time && time < Keyframes[i + 1].time)
    {
      CurrFrame = i;
      break;
    }
  }
}

void BoneAnimation::DrawDebugMenu()
{
  ImGui::Text("Bone: [%d]%s", BoneID, BoneName.c_str());
  ImGui::Text("Current Frame: %d", CurrFrame);

  if (ImGui::CollapsingHeader((std::string("Keyframes##") + std::to_string(BoneID)).c_str()))
  {
    for (unsigned int i = 0; i < Keyframes.size(); i++)
    {
      ImGui::Text("[%d]: time: %f", i, Keyframes[i].time);
      
      const glm::vec3 &pos = Keyframes[i].value.GetPosition();
      const glm::vec3 &scl = Keyframes[i].value.GetScale();
      const Quaternion &rot = Keyframes[i].value.GetRotation();
      const glm::vec4& rotVec = rot.GetVec();

      ImGui::Text("Position: [%f, %f, %f]", pos.x, pos.y, pos.z);
      ImGui::Text("Scale: [%f, %f, %f]", scl.x, scl.y, scl.z);
      ImGui::Text("Rotation: [%fi, %fj, %fk, %f]", rotVec.x, rotVec.y, rotVec.z, rotVec.w);
      ImGui::Separator();
    }
  }
}

void SkeletalAnimation::DrawDebugMenu()
{
  ImGui::Text("Name: %s", Name.c_str());
  ImGui::Text("Duration: %f", Duration);
  ImGui::Text("TickPerSec: %f", TickPerSec);

  std::string loopStr("Loop##");
  loopStr += Name;
  ImGui::Checkbox(loopStr.c_str(), &Loop);

  if (ImGui::CollapsingHeader((std::string("Unused Bones##") + Name).c_str()))
  {
    for (unsigned int b : UnunsedBones)
    {
      ImGui::Text("[%d], ", b);

      if(b != UnunsedBones.back())
        ImGui::SameLine();
    }
  }
  
  if (ImGui::CollapsingHeader((std::string("Bone Animations##") + Name).c_str()))
  {
    for (BoneAnimation& boneAnim : BoneAnimations)
    {
      boneAnim.DrawDebugMenu();
      ImGui::Separator();
    }
  }
}

void SkeletalAnimationSet::SetAnimation(int newAnim)
{
  CurrAnimation = newAnim; 
  Reset();

  std::vector<Bone> &bones = Parent->GetBones();
  for (unsigned int unusedBone : Animations[CurrAnimation].UnunsedBones)
  {
    bones[unusedBone].AnimationMatrix = bones[unusedBone].BindposeMatrix;
  }
}

void SkeletalAnimationSet::Start()
{ 
  Playing = true; 
  Reset();

  Parent->SetUseAnimation(true);
}

void SkeletalAnimationSet::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("Skeletal Animations"))
  {
    ImGui::Separator();
    ImGui::Indent(10.f);

    static bool selectingMesh;
    if(ImGui::Button("Select mesh"))
    {
      selectingMesh = true;
    }
    if (selectingMesh)
    {
      ResourceManager::DrawResourceSelector<SkinnedMesh>(&Parent, &selectingMesh);
      if (selectingMesh == false && Parent)
      {
        Animations = Parent->GetAnimations();
      }
    }

    if (ImGui::Button("Restart"))
    {
      Start();
    }
    ImGui::SameLine();
    if (Playing)
    {
      if (ImGui::Button("Stop"))
      {
        Stop();
      }
    }
    else
    {
      if (ImGui::Button("Resume"))
      {
        Resume();
      }
    }

    ImGui::Text("Play Status: %s", Playing ? "Playing" : "Stop");
    if (CurrAnimation > -1)
    {
      ImGui::Text("Current Animation: %s", Animations[CurrAnimation].Name.c_str());
    }
    else
    {
      ImGui::Text("Current Animation: %s", "none");
    }

    ImGui::Text("Current Time: %f", CurrentTime);
    ImGui::Text("Current Frame: %d", CurrFrame);
    ImGui::SliderFloat("Animation Speed", &PlaySpeed, 0.1f, 30.f);

    ImGui::Text("Custom Time: %f", CustomTime);
    ImGui::Checkbox("Use custom time", &UseCustomTime);

    if (ImGui::CollapsingHeader("Animation List"))
    {
      ImGui::Indent(10.f);

      unsigned int animID = 0;
      std::string useStr = "Use##";
      for (SkeletalAnimation& animation : Animations)
      {
        animation.DrawDebugMenu();

        if (ImGui::Button((useStr + std::to_string(animID)).c_str()))
        {
          SetAnimation(animID);
        }
        animID++;

        ImGui::Separator();
      }

      ImGui::Unindent(10.f);
    }

    ImGui::Unindent(10.f);
    ImGui::Separator();
  }
}