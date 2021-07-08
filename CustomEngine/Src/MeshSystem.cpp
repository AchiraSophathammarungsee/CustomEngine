#include "MeshSystem.h"
#include "Coordinator.h"
#include "Transform.h"
#include "shader.hpp"
#include "Material.h"
#include "Model.h"
#include "MeshComp.h"
#include "Renderer.h"

void MeshSystem::Draw()
{
  for (EntityID entity : Entities)
  {
    auto& transform = Coordinator::GetComponent<Transform>(entity);
    auto& mesh = Coordinator::GetComponent<MeshComp>(entity);

    if (transform.NeedUpdate())
    {
      transform.UpdateMatrix();
      mesh.mesh->boundingBox.UpdateTransform(transform, mesh.BB.BoxVertices);
    }
    mesh.boundingVolume.UpdateVolume(transform.GetPosition(), mesh.BB.BoxVertices);

    Renderer::AddRenderObj(entity, mesh.mesh->GetVAO(), mesh.mesh->GetIndiceCount(), mesh.mesh->GetPrimitiveMode(), transform, mesh.material, mesh.boundingVolume);
  }
}