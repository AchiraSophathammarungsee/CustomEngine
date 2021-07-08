#pragma once
#ifndef CLOTHSYSTEM_H
#define CLOTHSYSTEM_H

#include "System.h"

class Shader;

class ClothSystem : public System
{
public:
  ClothSystem() {};

  void Init();
  void Update();
  void Draw();

private:

};

#endif