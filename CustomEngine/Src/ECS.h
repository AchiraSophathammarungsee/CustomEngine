
#ifndef ECS_H
#define ECS_H

#include "EngineCore.h"
#include <cstdint>
#include <bitset>

typedef uint32_t EntityID;
const EntityID MAX_ENTITIES = 5000;

typedef uint32_t ComponentType;
const ComponentType MAX_COMPONENTS = 32;

typedef std::bitset<MAX_COMPONENTS> Signature;

#endif