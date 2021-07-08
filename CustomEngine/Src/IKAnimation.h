#pragma once
#ifndef IKANIMATION_H
#define IKANIMATION_H

#include <glm/glm.hpp>

class SkinnedMeshComp;

class IKAnimation
{
public:
  IKAnimation(int endbone = -1) 
    : EndEffectorBone(endbone), ChainLength(2), Tolerance(0.1f), MinDistProgress(0.01f), Manipulator(0.f), Influence(1.f), AnimDuration(5.f), AnimTimer(0.f), Playing(false), Mesh(nullptr) {};
  ~IKAnimation() {};

  void Play();
  void Stop();
  void DrawDebugMenu();
   
  int EndEffectorBone;
  int ChainLength;
  float Tolerance;
  float MinDistProgress;
  glm::vec3 Manipulator; // target
  glm::vec3 InitEndEffectPos;
  float Influence; // overwrite multiplier
  float AnimDuration;
  float AnimTimer;
  bool Playing;
  SkinnedMeshComp* Mesh;
private:
  
};

#endif