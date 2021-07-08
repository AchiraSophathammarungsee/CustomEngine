#include "ComponentManager.h"

std::unordered_map<const char*, ComponentType> ComponentManager::ComponentTypes;
std::shared_ptr<IComponentArray> ComponentManager::ComponentArrays[MAX_COMPONENT_ARRAY_NUMBER];
ComponentType ComponentManager::NextComponentType;