
#ifndef GRAVITY_H
#define GRAVITY_H

#include "System.h"

class Gravity : public System
{
public:
  Gravity() : GravityConstant(0.00981f) {};

  void Update();

  void DrawDebugMenu();

private:
  float GravityConstant;
};

#endif