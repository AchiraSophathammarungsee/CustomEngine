#pragma once
#ifndef SKINNEDMESHSYSTEM_H
#define SKINNEDMESHSYSTEM_H

#include "System.h"
#include "BoneData.h"

class Shader;

class SkinnedMeshSystem : public System
{
public:
  SkinnedMeshSystem() {};

  void Init();
  void Draw();

private:
  void CalculateBoneMatrices(bool useAnimation, std::vector<Bone>& bones, Bone* node, std::vector<glm::mat4> &finalMatArray);
};

#endif