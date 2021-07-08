
#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <queue>
#include <array>
#include "ECS.h"
#include "Rect.h"
#include "EditorObject.h"
#include "Entity.h"

class EntityManager
{
public:
  static void Init();

  static EntityID CreateEntity(const std::string &name);
  static void DestroyEntity(EntityID entity);

  static void SetSignature(EntityID entity, Signature signature);
  static Signature GetSignature(EntityID entity);

  static void DrawDebugMenu(Rect<unsigned int> window);
  static void DrawEntityEditor(Rect<unsigned int> window);

private:
  EntityManager() {};
  ~EntityManager() {};

  static std::queue<EntityID> AvailableEntities;
  static std::array<Signature, MAX_ENTITIES> Signatures;
  static uint32_t LivingEntityCount;

  static EditObjects<MAX_ENTITIES> objNames;
  static EntityID editEntity;
  static char editEntityNameBuffer[256];
};

#endif