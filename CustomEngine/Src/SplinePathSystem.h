#pragma once
#ifndef SPLINEPATHSYSTEM_H
#define SPLINEPATHSYSTEM_H

#include "System.h"

class Shader;

class SplinePathSystem : public System
{
public:
  SplinePathSystem() {};

  void Update();
  void Draw(Shader* shader);

private:
};

#endif