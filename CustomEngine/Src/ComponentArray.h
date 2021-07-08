
#ifndef COMPONENTARRAY_H
#define COMPONENTARRAY_H

#include "ECS.h"
#include <unordered_map>
#include "Log.h"

typedef void (*InspecFunc)();

class IComponentArray
{
public:
  virtual ~IComponentArray() = default;
  virtual void AddDefaultComponent(EntityID entity) = 0;
  virtual void RemoveComponent(EntityID entity) = 0;
  virtual void EntityDestroyed(EntityID entity) = 0;

  virtual void DrawDebugMenu(EntityID entity) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray
{
public:

  void InsertData(EntityID entity, const T &component)
  {
    ASSERT(EntityToIndex.find(entity) == EntityToIndex.end(), "[ComponentArray]: Tried to add component to same entity with id: {}.", entity);

    size_t newIndex = Size;
    EntityToIndex[entity] = newIndex;
    IndexToEntity[newIndex] = entity;
    Components[newIndex] = component;
    Size++;
  }

  void AddDefaultComponent(EntityID entity) override
  {
    InsertData(entity, T());
  }

  void RemoveData(EntityID entity)
  {
    ASSERT(EntityToIndex.find(entity) != EntityToIndex.end(), "[ComponentArray]: Attempted to remove component in non-existent entity: {}.", entity);

    // Copy element at end into deleted element's place to maintain density (no invalid/empty slot)
    size_t removeID = EntityToIndex[entity];
    size_t lastElementID = Size - 1;
    Components[removeID] = Components[lastElementID];

    // Update map to point to moved spot
    EntityID lastElementEntity = IndexToEntity[lastElementID];
    EntityToIndex[lastElementEntity] = removeID;
    IndexToEntity[removeID] = lastElementEntity;

    EntityToIndex.erase(entity);
    IndexToEntity.erase(lastElementID);

    Size--;
  }

  void RemoveComponent(EntityID entity) override
  {
    RemoveData(entity);
  }

  T& GetData(EntityID entity)
  {
    ASSERT(EntityToIndex.find(entity) != EntityToIndex.end(), "[ComponentArray]: Tried to retrieve non-existent component from entity {}.", entity);

    return Components[EntityToIndex[entity]];
  }

  void EntityDestroyed(EntityID entity) override
  {
    if (EntityToIndex.find(entity) != EntityToIndex.end())
    {
      RemoveData(entity);
    }
  }

  void DrawDebugMenu(EntityID entity) override
  {
    if (EntityToIndex.find(entity) != EntityToIndex.end())
    {
      Components[EntityToIndex[entity]].DrawDebugMenu();
    }
  }

private:

  T Components[MAX_ENTITIES];

  std::unordered_map<EntityID, size_t> EntityToIndex;
  std::unordered_map<size_t, EntityID> IndexToEntity;
  size_t Size;
};

#endif