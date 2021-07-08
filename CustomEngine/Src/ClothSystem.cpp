#include "ClothSystem.h"
#include "shader.hpp"
#include "Coordinator.h"
#include "ClothComp.h"
#include "Material.h"
#include "Time.h"
#include "Renderer.h"
#include "ColliderManager.h"
#include "Collider.h"

static glm::vec3 SpringColors[ClothSpring::ST_COUNT] = { glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 1.f) };

void ClothSystem::Init()
{
  for (EntityID entity : Entities)
  {
    auto& cloth = Coordinator::GetComponent<ClothComp>(entity);

    cloth.GenerateMesh();
    cloth.RegisterMesh();
  }
}

void ClothSystem::Update()
{
  for (EntityID entity : Entities)
  {
    auto& cloth = Coordinator::GetComponent<ClothComp>(entity);
    cloth.timer += Time::DT();

    // update spring current lengths
    for (ClothSpring& spring : cloth.ClothSprings)
    {
      spring.currLength = glm::distance(cloth.ClothVertices[spring.vertexIDs[0]].position, cloth.ClothVertices[spring.vertexIDs[1]].position);

      if(cloth.ShowDebug) Renderer::DebugLine(cloth.ClothVertices[spring.vertexIDs[0]].position, cloth.ClothVertices[spring.vertexIDs[1]].position, 1.f, SpringColors[spring.type]);
    }

    // update vertices
    float vertMass = cloth.Mass;
    for (int i = 0; i < cloth.ClothVertices.size(); i++)
    {
      ClothVertex& vert = cloth.ClothVertices[i];
      if (vert.pin == false)
      {
        glm::vec3 springForce(0.f);
        //glm::vec3 dampForce(0.f);
        for (SpringConnect connection : vert.springConnections)
        {
          ClothSpring& spring = cloth.ClothSprings[connection.springID];
          ClothVertex& otherVert = cloth.ClothVertices[connection.otherVertID];

          glm::vec3 posDiff = vert.position - otherVert.position;
          glm::vec3 velDiff = vert.velocity - otherVert.velocity;
          glm::vec3 dir = (posDiff == glm::vec3(0.f)) ? posDiff : glm::normalize(posDiff);

          springForce = -cloth.SpringConstant[spring.type] * (spring.currLength - spring.restLength) * dir;
          //dampForce = cloth.DampConstant * glm::dot(velDiff, posDiff) * dir;
          vert.force += springForce;// +dampForce;
          //otherVert.force -= springForce;
        }

        glm::vec3 mg = vertMass * glm::vec3(0.f, -cloth.Gravity, 0.f);
        glm::vec3 dampForce = -vert.velocity * cloth.DampConstant;

        glm::vec3 windForce = (glm::vec3(glm::sin(cloth.timer * 2.f) * 0.5f + 0.5f) + 0.25f * glm::vec3(glm::sin(cloth.timer * 4.f))) * cloth.Wind;

        vert.force += mg + windForce + dampForce;
        vert.acceleration = vert.force / vertMass;
        
        glm::vec3 tmp2 = vert.prevPos;
        glm::vec3 tmp = vert.position;
        vert.position = 2.f * vert.position - vert.prevPos + vert.acceleration * Time::FixedDT() * Time::FixedDT();
        vert.velocity = vert.position - vert.prevPos;
        // cloth-cloth collsion
        /*for (int j = i + 1; j < cloth.ClothVertices.size(); j++)
        {
          ClothVertex& otherVert = cloth.ClothVertices[j];

          float radius = ;
        }*/

        vert.prevPos = tmp;
        vert.force = glm::vec3(0.f);

        // cloth-object collision
        CollisionInfo info = ColliderManager::PointTest(vert.position);
        if (info.Hit)
        {
          vert.position = tmp;
          //vert.prevPos = tmp2;
        }
      }

      if (cloth.ShowDebug)
      {
        Transform point(vert.position, glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.02f));
        Renderer::DebugLineCube(point, glm::vec3(1.f, 1.f, 0.f));
      }
    }
  }
}

void ClothSystem::Draw()
{
  for (EntityID entity : Entities)
  {
    auto& cloth = Coordinator::GetComponent<ClothComp>(entity);
    cloth.UpdateMesh();

    Renderer::AddRenderObj(entity, cloth.GetVAO(), cloth.GetIndiceCount(), cloth.GetPrimitiveMode(), Transform(), cloth.material);
  }
  
  /*shader->use();
  glDisable(GL_CULL_FACE);
  for (EntityID entity : Entities)
  {
    auto& cloth = Coordinator::GetComponent<ClothComp>(entity);

    shader->setMat4("modelMatrix", glm::mat4(1.f));

    if (drawMat)
    {
      cloth.material->UpdateUniform(shader);
    }

    cloth.UpdateMesh();
    cloth.Draw();
  }
  glEnable(GL_CULL_FACE);*/
}