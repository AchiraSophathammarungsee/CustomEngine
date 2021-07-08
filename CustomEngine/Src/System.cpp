#include "System.h"

void System::InitSystem()
{
  std::memset(ValidEntities, -1, sizeof(ValidEntities));
}

void System::AddEntity(EntityID entity)
{
  if (ValidEntities[entity] < 0)
  {
    ValidEntities[entity] = Entities.size();
    Entities.push_back(entity);
  }
}

bool System::HaveEntity(EntityID entity)
{
  return ValidEntities[entity] > -1;
}

void System::RemoveEntity(EntityID entity)
{
  if (ValidEntities[entity] > -1)
  {
    // swap removed entity with the back entity, pop back the removed entity
    Entities[ValidEntities[entity]] = Entities.back();
    ValidEntities[Entities.back()] = ValidEntities[entity];
    ValidEntities[entity] = -1;

    Entities.pop_back();
  }
}