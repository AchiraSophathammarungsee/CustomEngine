
#ifndef SYSTEM_H
#define SYSTEM_H

#include "ECS.h"
#include <set>

class System
{
public:
  void InitSystem();

  void AddEntity(EntityID entity);
  bool HaveEntity(EntityID entity);
  void RemoveEntity(EntityID entity);

protected:
  std::vector<EntityID> Entities;
  int ValidEntities[MAX_ENTITIES];
  //std::set<EntityID> Entities;
};

#endif