
#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "ComponentArray.h"

#define MAX_COMPONENT_ARRAY_NUMBER 256

class ComponentManager
{
public:
  template<typename T>
  static void RegisterComponent()
  {
    const char* typeName = typeid(T).name();

    ASSERT(ComponentTypes.find(typeName) == ComponentTypes.end(), "[ComponentManager] Attempt to register component of type: {} twice.", typeName);

    ComponentTypes.insert({ typeName, NextComponentType });
    ComponentArrays[NextComponentType] = std::make_shared<ComponentArray<T>>();

    NextComponentType++;

    LOG_TRACE_S("ComponentManager", "Register component: {}", typeName);
  };

  template<typename T>
  static ComponentType GetComponentType()
  {
    const char* typeName = typeid(T).name();

    ASSERT(ComponentTypes.find(typeName) != ComponentTypes.end(), "[ComponentManager] Tried to get unregisted component: {}.", typeName);

    // Return this component's type - used for creating signatures
    return ComponentTypes[typeName];
  };

  template<typename T>
  static void AddComponent(EntityID entity, const T &component)
  {
    GetComponentArray<T>()->InsertData(entity, component);
  };

  template<typename T>
  static void RemoveComponent(EntityID entity)
  {
    GetComponentArray<T>()->RemoveData(entity);
  };

  template<typename T>
  static T& GetComponent(EntityID entity)
  {
    return GetComponentArray<T>()->GetData(entity);
  };

  static void EntityDestroyed(EntityID entity)
  {
    // Notify each component array that an entity has been destroyed
    // If it has a component for that entity, it will remove it
    for (ComponentType type = 0; type < NextComponentType; type++)
    {
      ComponentArrays[type]->EntityDestroyed(entity);
    }
  };

private:
  // Map from type string pointer to a component type
  static std::unordered_map<const char*, ComponentType> ComponentTypes;

  // Static arrays of ComponentType to ComponentArray
  static std::shared_ptr<IComponentArray> ComponentArrays[MAX_COMPONENT_ARRAY_NUMBER];

  // The component type to be assigned to the next registered component - starting at 0
  static ComponentType NextComponentType;

  template<typename T>
  static std::shared_ptr<ComponentArray<T>> GetComponentArray()
  {
    const char* typeName = typeid(T).name();

    auto compType = ComponentTypes.find(typeName);

    ASSERT(compType != ComponentTypes.end(), "[ComponentManager] Attempt to get component array: {} before register.", typeName);

    return std::static_pointer_cast<ComponentArray<T>>(ComponentArrays[compType->second]);
  };

  friend class EntityManager;
};

#endif