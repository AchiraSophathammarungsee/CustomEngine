#include "Coordinator.h"

void Coordinator::Init()
{
  LOG_TRACE("Coordinator", "Initializing...");

  EntityManager::Init();
}

EntityID Coordinator::CreateEntity(const std::string &name)
{
  return EntityManager::CreateEntity(name);
}

void Coordinator::DestroyEntity(EntityID entity)
{
  LOG_TRACE_S("Coordinator", "Destroying entity {}...", entity);

  EntityManager::DestroyEntity(entity);
  ComponentManager::EntityDestroyed(entity);
  SystemManager::EntityDestroyed(entity);
}