
#ifndef COORDINATOR_H
#define COORDINATOR_H

#include "SystemManager.h"
#include "EntityManager.h"
#include "ComponentManager.h"

class Coordinator
{
public:
  static void Init();
  static EntityID CreateEntity(const std::string& name);
  static void DestroyEntity(EntityID entity);

  template<typename T>
  static void RegisterComponent()
  {
    ComponentManager::RegisterComponent<T>();
  }

  template<typename T>
  static void AddComponent(EntityID entity, const T &component)
  {
    ComponentManager::AddComponent<T>(entity, component);

    Signature sign = EntityManager::GetSignature(entity);
    sign.set(ComponentManager::GetComponentType<T>(), true);
    EntityManager::SetSignature(entity, sign);

    SystemManager::EntitySignatureChanged(entity, sign);
  }

  template<typename T>
  static void RemoveComponent(EntityID entity)
  {
    ComponentManager::RemoveComponent<T>(entity);

    Signature sign = EntityManager::GetSignature(entity);
    sign.set(ComponentManager::GetComponentType<T>(), false);
    EntityManager::SetSignature(entity, sign);

    SystemManager::EntitySignatureChanged(entity, sign);
  }

  template<typename T>
  static T& GetComponent(EntityID entity)
  {
    return ComponentManager::GetComponent<T>(entity);
  }

  template<typename T>
  static ComponentType GetComponentType()
  {
    return ComponentManager::GetComponentType<T>();
  }


  // System methods
  template<typename T>
  static std::shared_ptr<T> RegisterSystem()
  {
    return SystemManager::RegisterSystem<T>();
  }

  template<typename T>
  static void SetSystemSignature(Signature signature)
  {
    SystemManager::SetSignature<T>(signature);
  }

private:

};

#endif