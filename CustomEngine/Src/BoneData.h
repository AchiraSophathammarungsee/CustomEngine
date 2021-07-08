#pragma once
#ifndef BONEDATA_H
#define BONEDATA_H

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "SkeletalAnimation.h"

struct Bone
{
  Bone() : ID(0), Name(), Parent(-1), OffsetMatrix(1.f), BindposeMatrix(1.f), AnimationMatrix(1.f), TransformMatrix(1.f), FinalMatrix(1.f), IKTransform(1.f), UseAnimation(false) {};

  unsigned int ID;
  std::string Name;
  int Parent;
  std::vector<unsigned int> Childrens;
  glm::mat4 OffsetMatrix;
  glm::mat4 BindposeMatrix;
  glm::mat4 AnimationMatrix;
  glm::mat4 TransformMatrix;
  glm::mat4 FinalMatrix;
  glm::mat4 IKTransform;
  bool UseAnimation;
};

#endif