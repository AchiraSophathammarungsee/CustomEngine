#include "EntityManager.h"
#include "EngineCore.h"
#include "Editor.h"
#include "Coordinator.h"
#include "Transform.h"

std::queue<EntityID> EntityManager::AvailableEntities;
std::array<Signature, MAX_ENTITIES> EntityManager::Signatures;
uint32_t EntityManager::LivingEntityCount;

EditObjects<MAX_ENTITIES> EntityManager::objNames;
EntityID EntityManager::editEntity;
char EntityManager::editEntityNameBuffer[256];

void EntityManager::Init()
{
  LOG_TRACE("EntityManager", "Initializing...");

  for (EntityID id = 0; id < MAX_ENTITIES; id++)
  {
    AvailableEntities.push(id);
  }
}

EntityID EntityManager::CreateEntity(const std::string& name)
{
  if (LivingEntityCount >= MAX_ENTITIES)
  {
    LOG_ERROR("EntityManager", "Failed to create new entity: FULL CAPACITY reached!");
    return 0;
  }

  EntityID entity = AvailableEntities.front();
  AvailableEntities.pop();
  LivingEntityCount++;

  std::string newname = name + " [" + std::to_string(LivingEntityCount) + ']';
  objNames.SetName(entity, newname.c_str());
  LOG_TRACE_S("EntityManager", "Created new entity with id: {}", entity);

  return entity;
}

void EntityManager::DestroyEntity(EntityID entity)
{
  if (entity >= MAX_ENTITIES)
  {
    LOG_ERROR_S("EntityManager", "Attempt to destroy invalid entity with id: {}", entity);
    return;
  }

  // clear destroyed entity's signature
  Signatures[entity].reset();

  // add available slot back
  AvailableEntities.push(entity);
  LivingEntityCount--;

  LOG_TRACE_S("EntityManager", "Destroying entity id: {}", entity);
}

void EntityManager::SetSignature(EntityID entity, Signature signature)
{
  if (entity >= MAX_ENTITIES)
  {
    LOG_ERROR_S("EntityManager", "Attempt to set signature of an invalid entity with id: {}", entity);
    return;
  }

  Signatures[entity] = signature;
}

Signature EntityManager::GetSignature(EntityID entity)
{
  ASSERT(entity < MAX_ENTITIES, "Component not registered before use.");

  return Signatures[entity];
}

void EntityManager::DrawDebugMenu(Rect<unsigned int> window)
{
  static std::stringstream ss;

  ImGui::Text("Living entity count: %d", LivingEntityCount);
  if (ImGui::Button("Create entity"))
  {
    EntityID entity = CreateEntity("unnamed");
    Coordinator::AddComponent(entity, Transform());
  }
  if (ImGui::CollapsingHeader("Entities List"))
  {
    for (size_t id = 0; id < LivingEntityCount; id++)
    {
      ss.str(""); ss.clear();
      ss << "Entity[" << id << "] " << objNames.GetName(id);
      if (ImGui::Button(ss.str().c_str()))
      {
        Editor::SetObjEditor(DrawEntityEditor);
        editEntity = id;
        strcpy_s(editEntityNameBuffer, objNames.GetName(editEntity).c_str());
      }

      ImGui::SameLine();
      ss.str(""); ss.clear();
      ss << "Delete Entity##" << id;// objNames.GetName(id);
      if (ImGui::Button(ss.str().c_str()))
      {
        Coordinator::DestroyEntity(id);
      }
    }
  }
}

void EntityManager::DrawEntityEditor(Rect<unsigned int> window)
{
  if (ImGui::InputText("Entity Name", editEntityNameBuffer, 256))
  {
    objNames.SetName(editEntity, editEntityNameBuffer);
  }
  ImGui::Text("ID[%d]: signature = %s", editEntity, Signatures[editEntity].to_string().c_str());

  static bool selectComp = false;
  static bool removeComp = false;
  if (ImGui::Button("Add Component"))
  {
    selectComp = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("Remove Component"))
  {
    removeComp = true;
  }

  Signature sign = GetSignature(editEntity);
  for (ComponentType type = 0; type < ComponentManager::NextComponentType; type++)
  {
    if (sign.test(type))
    {
      ComponentManager::ComponentArrays[type]->DrawDebugMenu(editEntity);
    }
  }

  if (selectComp)
  {
    ImGui::SetNextWindowSize(ImVec2(200, 300));
    ImGui::Begin("Select Component", &selectComp);

    for (auto pair : ComponentManager::ComponentTypes)
    {
      if (ImGui::Button(pair.first))
      {
        ComponentManager::ComponentArrays[pair.second]->AddDefaultComponent(editEntity);

        Signature sign = EntityManager::GetSignature(editEntity);
        sign.set(pair.second, true);
        EntityManager::SetSignature(editEntity, sign);

        SystemManager::EntitySignatureChanged(editEntity, sign);

        selectComp = false;
      }
    }

    ImGui::End();
  }
  
  if (removeComp)
  {
    ImGui::SetNextWindowSize(ImVec2(200, 300));
    ImGui::Begin("Remove Component", &removeComp);
    for (auto pair : ComponentManager::ComponentTypes)
    {
      if (sign.test(pair.second))
      {
        if (ImGui::Button(pair.first))
        {
          ComponentManager::ComponentArrays[pair.second]->RemoveComponent(editEntity);

          Signature sign = EntityManager::GetSignature(editEntity);
          sign.set(pair.second, false);
          EntityManager::SetSignature(editEntity, sign);
          SystemManager::EntitySignatureChanged(editEntity, sign);

          removeComp = false;
        }
      }
    }

    ImGui::End();
  }
}