#include "SkinnedMeshSystem.h"
#include "Coordinator.h"
#include "Transform.h"
#include "SkinnedMeshComp.h"
#include "shader.hpp"
#include "Material.h"
#include "SkinnedMesh.h"
#include "Renderer.h"

void SkinnedMeshSystem::CalculateBoneMatrices(bool useAnimation, std::vector<Bone> &bones, Bone *node, std::vector<glm::mat4>& finalMatArray)
{
  const glm::mat4 &boneMat = node->BindposeMatrix;

  if (node->Parent == -1)
  {
    node->TransformMatrix = boneMat;
  }
  else
  {
    node->TransformMatrix = bones[node->Parent].TransformMatrix * boneMat;
  }

  node->FinalMatrix = node->TransformMatrix * node->OffsetMatrix;

  finalMatArray[node->ID] = node->FinalMatrix;

  for (unsigned int child : node->Childrens)
  {
    CalculateBoneMatrices(useAnimation, bones, &bones[child], finalMatArray);
  }
}

void SkinnedMeshSystem::Init()
{
  for (EntityID entity : Entities)
  {
    auto& mesh = Coordinator::GetComponent<SkinnedMeshComp>(entity);
    std::vector<Bone>& bones = mesh.mesh->GetBones();

    CalculateBoneMatrices(mesh.mesh->IsUseAnimation(), bones, &bones[0], mesh.mesh->GetFinalMatArray());
  }
}

void SkinnedMeshSystem::Draw()
{
  for (EntityID entity : Entities)
  {
    auto& transform = Coordinator::GetComponent<Transform>(entity);
    auto& mesh = Coordinator::GetComponent<SkinnedMeshComp>(entity);

    std::vector<Bone>& bones = mesh.mesh->GetBones();
    auto& finalArray = mesh.mesh->GetFinalMatArray();

    if (mesh.mesh->IsUseAnimation() == false && mesh.mesh->IsUseIK() == false)
    {
      CalculateBoneMatrices(mesh.mesh->IsUseAnimation(), bones, &bones[0], finalArray);
    }
    
    if (mesh.mesh->ShowBones())
    {
      for (unsigned int i = 0; i < bones.size(); i++)
      {

        glm::mat4 debugMat = transform.GetMatrix() * glm::scale(bones[i].TransformMatrix, glm::vec3(0.1f));

        Transform boneTrm;
        boneTrm.SetMatrix(debugMat);
        Renderer::DebugLineCube(boneTrm, glm::vec3(1.f, 1.f, 0.f));

        glm::vec4 start = debugMat * glm::vec4(0.f, 0.f, 0.f, 1.f);

        glm::vec4 end;
        if (bones[i].Parent == -1)
        {
          end = start + glm::vec4(0.f, 1.f, 0.f, 0.f);
        }
        else
        {
          glm::mat4 parentMat = transform.GetMatrix() * bones[bones[i].Parent].TransformMatrix;
          end = parentMat * glm::vec4(0.f, 0.f, 0.f, 1.f);
        }

        Renderer::DebugLine(glm::vec3(start.x, start.y, start.z), glm::vec3(end.x, end.y, end.z), 1.f, glm::vec3(0.f, 1.f, 1.f));

        //std::string arrayStr = "bones[" + std::to_string(i) + "].";
        //shader->setMat4Array(arrayStr, mesh.mesh->getInverseRoot() * bones[i].FinalMatrix, bones.size());
      }
    }

    Renderer::AddRenderObj(entity, mesh.mesh->GetVAO(), mesh.mesh->GetIndiceCount(), mesh.mesh->GetPrimitiveMode(), transform, mesh.material, &finalArray);
  }

  /*shader->use();
  for (EntityID entity : Entities)
  {
    auto& transform = Coordinator::GetComponent<Transform>(entity);
    auto& mesh = Coordinator::GetComponent<SkinnedMeshComp>(entity);

    transform.UpdateMatrix();
    shader->setMat4("modelMatrix", transform.GetMatrix());

    std::vector<Bone>& bones = mesh.mesh->GetBones();

    if (mesh.mesh->IsUseAnimation() == false && mesh.mesh->IsUseIK() == false)
    {
      CalculateBoneMatrices(mesh.mesh->IsUseAnimation(), bones, &bones[0]);
    }

    for (unsigned int i = 0; i < bones.size(); i++)
    {
      if (mesh.mesh->ShowBones())
      {
        glm::mat4 debugMat = transform.GetMatrix() * glm::scale(bones[i].TransformMatrix, glm::vec3(0.1f));

        Transform boneTrm;
        boneTrm.SetMatrix(debugMat);
        Renderer::DebugLineCube(boneTrm, glm::vec3(1.f, 1.f, 0.f));

        glm::vec4 start = debugMat * glm::vec4(0.f, 0.f, 0.f, 1.f);

        glm::vec4 end;
        if (bones[i].Parent == -1)
        {
          end = start + glm::vec4(0.f, 1.f, 0.f, 0.f);
        }
        else
        {
          glm::mat4 parentMat = transform.GetMatrix() * bones[bones[i].Parent].TransformMatrix;
          end = parentMat * glm::vec4(0.f, 0.f, 0.f, 1.f);
        }
        
        Renderer::DebugLine(glm::vec3(start.x, start.y, start.z), glm::vec3(end.x, end.y, end.z), 1.f, glm::vec3(0.f, 1.f, 1.f));
      }

      std::string arrayStr = "bones[" + std::to_string(i) + "].";
      shader->setMat4(arrayStr, mesh.mesh->getInverseRoot() * bones[i].FinalMatrix);
    }

    if (drawMat)
    {
      mesh.material->UpdateUniform(shader);
    }
    mesh.mesh->Draw();
  }*/
}