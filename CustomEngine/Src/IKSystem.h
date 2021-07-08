#pragma once
#ifndef IKSYSTEM_H
#define IKSYSTEM_H

#include "System.h"
#include "SkinnedMeshComp.h"
#include "IKAnimation.h"
#include "Transform.h"

class IKSystem : public System
{
public:
  IKSystem() {};
  ~IKSystem() {};

  void Init();
  void Update();

private:
  void CCD(Transform &transform, SkinnedMeshComp &mesh, IKAnimation &ik);
};

#endif