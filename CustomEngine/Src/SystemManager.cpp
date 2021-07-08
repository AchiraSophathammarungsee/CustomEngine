#include "SystemManager.h"

std::unordered_map<const char*, Signature> SystemManager::Signatures;
std::unordered_map<const char*, std::shared_ptr<System>> SystemManager::Systems;