#include "SkeletalAnimator.h"
#include "Coordinator.h"
#include "SkeletalAnimation.h"
#include "SkinnedMeshComp.h"
#include "SkinnedMesh.h"
#include "Time.h"
#include "Vec.h"

void UpdateBoneMatrix(std::vector<Bone> &bones, Bone *currBone, int level, std::vector<glm::mat4>& finalMat)
{
  if (currBone->Parent == -1)
  {
    currBone->TransformMatrix = currBone->AnimationMatrix;
  }
  else
  {
    currBone->TransformMatrix = bones[currBone->Parent].TransformMatrix * currBone->AnimationMatrix;
  }

  currBone->FinalMatrix = currBone->TransformMatrix * currBone->OffsetMatrix;

  finalMat[currBone->ID] = currBone->FinalMatrix;
  /*std::string indent;
  std::string matIndent;

  for (int i = 0; i < level; i++)
    indent += '.';

  for (int i = 0; i < 1 + currBone->Name.size() + 3; i++)
    matIndent += ' ';

  std::cout << indent << '[' << currBone->Name << "]: [" << currBone->TransformMatrix[0][0] << ", " << currBone->TransformMatrix[0][1] << ", " << currBone->TransformMatrix[0][2] << ", " << currBone->TransformMatrix[0][3] << ']' << std::endl;
  std::cout << indent << matIndent << '[' << currBone->TransformMatrix[1][0] << ", " << currBone->TransformMatrix[1][1] << ", " << currBone->TransformMatrix[1][2] << ", " << currBone->TransformMatrix[1][3] << ']' << std::endl;
  std::cout << indent << matIndent << '[' << currBone->TransformMatrix[2][0] << ", " << currBone->TransformMatrix[2][1] << ", " << currBone->TransformMatrix[2][2] << ", " << currBone->TransformMatrix[2][3] << ']' << std::endl;
  std::cout << indent << matIndent << '[' << currBone->TransformMatrix[3][0] << ", " << currBone->TransformMatrix[3][1] << ", " << currBone->TransformMatrix[3][2] << ", " << currBone->TransformMatrix[3][3] << ']' << std::endl;*/

  for (unsigned int child : currBone->Childrens)
  {
    UpdateBoneMatrix(bones, &bones[child], level + 1, finalMat);
  }
}

void SkeletalAnimator::Update()
{
  for (EntityID entity : Entities)
  {
    auto& mesh = Coordinator::GetComponent<SkinnedMeshComp>(entity);
    auto& animations = Coordinator::GetComponent<SkeletalAnimationSet>(entity);

    if (animations.Playing && animations.CurrAnimation > -1)
    {
      SkeletalAnimation& CurrAnimation = animations.Animations[animations.CurrAnimation];

      animations.CurrentTime += Time::DT() * animations.PlaySpeed;
      animations.CurrentTime = (animations.UseCustomTime) ? animations.CustomTime : animations.CurrentTime;

      if (animations.CurrentTime > CurrAnimation.Duration)
      {
        animations.CurrentTime = CurrAnimation.Duration;
        if (CurrAnimation.Loop)
        {
          animations.Start();
        }
        else
        {
          animations.Stop();
        }
      }

      /*std::cout << "----------------------------------------------------------------------------" << std::endl;
      std::cout << "TIME = " << animations.CurrentTime << std::endl;
      std::cout << "VQS" << std::endl;*/

      std::vector<Bone>& bones = mesh.mesh->GetBones();
      for (BoneAnimation& boneAnim : CurrAnimation.BoneAnimations)
      {
        VQS animVQS = boneAnim.Lerp(animations.CurrentTime);
        animVQS.UpdateMatrix();
        bones[boneAnim.BoneID].AnimationMatrix = animVQS.GetMatrix();

        /*std::cout << '[' << boneAnim.BoneName << "]: " << std::endl;
        std::cout << "  [pos]:   " << '[' << animVQS.GetPosition().x << ',' << animVQS.GetPosition().y << ',' << animVQS.GetPosition().z << ']' << std::endl;
        std::cout << "  [scale]: " << '[' << animVQS.GetScale().x << ',' << animVQS.GetScale().y << ',' << animVQS.GetScale().z << ']' << std::endl;
        std::cout << "  [rot]:   " << '[' << animVQS.GetRotation().GetVec().x << "i," << animVQS.GetRotation().GetVec().y << "j," << animVQS.GetRotation().GetVec().z << "k," << animVQS.GetRotation().GetVec().w << ']' << std::endl;*/
      }

      //std::cout << "TO WORLD MATRICES" << std::endl;
      std::vector<glm::mat4>& finalMat = animations.Parent->GetFinalMatArray();
      UpdateBoneMatrix(bones, &bones[0], 0, finalMat);
      //LOG_TRACE("SkeletalAnimator", "Calculating bone matrices");
    }
  }
}