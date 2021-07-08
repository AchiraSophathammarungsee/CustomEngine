
#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H

#include "ECS.h"
#include "System.h"
#include <unordered_map>

class SystemManager
{
public:
  template<typename T>
  static std::shared_ptr<T> RegisterSystem()
  {
    const char* typeName = typeid(T).name();

    ASSERT(Systems.find(typeName) == Systems.end(), "Registering system: {} more than once.", typeName);

    auto system = std::make_shared<T>();
    system->InitSystem();
    Systems.insert({ typeName, system });

    LOG_TRACE_S("SystemManager", "Register system: {}", typeName);

    return system;
  }

  template<typename T>
  static void SetSignature(Signature signature)
  {
    const char* typeName = typeid(T).name();

    ASSERT(Systems.find(typeName) != Systems.end(), "System {} used before registered.", typeName);

    Signatures.insert({ typeName, signature });

    LOG_TRACE_S("SystemManager", "Set system {} with signature: {}", typeName, signature);
  }

  static void EntityDestroyed(EntityID entity)
  {
    // Erase a destroyed entity from all system lists
    // Entities is a set so no check needed
    for (auto const& pair : Systems)
    {
      auto const& system = pair.second;
      //system->Entities.erase(entity);
      system->RemoveEntity(entity);
    }
  }

  static void EntitySignatureChanged(EntityID entity, Signature entitySignature)
  {
    LOG_TRACE_S("SystemManager", "Entity {} changed signature to: {}", entity, entitySignature);

    // Notify each system that an entity's signature changed
    for (auto const& pair : Systems)
    {
      auto const& type = pair.first;
      auto const& system = pair.second;
      auto const& systemSignature = Signatures[type];

      // Entity signature matches system signature - insert into set
      if ((entitySignature & systemSignature) == systemSignature)
      {
        //system->Entities.insert(entity);
        system->AddEntity(entity);
      }
      // Entity signature does not match system signature - erase from set
      else
      {
        //system->Entities.erase(entity);
        system->RemoveEntity(entity);
      }
    }
  }

private:
  // Map from system type string pointer to a signature
  static std::unordered_map<const char*, Signature> Signatures;

  // Map from system type string pointer to a system pointer
  static std::unordered_map<const char*, std::shared_ptr<System>> Systems;
};

#endif