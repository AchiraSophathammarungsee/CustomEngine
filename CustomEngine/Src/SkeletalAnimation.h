#pragma once
#ifndef SKELETALANIMATION_H
#define SKELETALANIMATION_H

#include <vector>
#include <glm/glm.hpp>
#include "VQS.h"
#include <string>

class SkinnedMesh;

template<typename T>
struct Keyframe
{
  Keyframe(const T& val = T(), double t = 0.0) : value(val), time(t) {};

  T value;
  double time;
};

struct BoneAnimation
{
public:
  BoneAnimation() : CurrFrame(0), BoneID(0) {};

  std::string BoneName;
  unsigned int BoneID;
  //std::vector<Keyframe<glm::vec3>> Positions;
  //std::vector<Keyframe<glm::vec3>> Scales;
  //std::vector<Keyframe<Quaternion>> Rotations;
  std::vector<Keyframe<VQS>> Keyframes;

  VQS Lerp(double time);

  void DrawDebugMenu();

private:
  unsigned int CurrFrame;
  void UpdateCurrFrame(double time);

  bool MaxFrame()
  {
    return CurrFrame >= (Keyframes.size() - 1);
  };
};

class SkeletalAnimation
{
public:
  std::string Name;
  double Duration;
  double TickPerSec;
  std::vector<BoneAnimation> BoneAnimations;
  std::vector<unsigned int> UnunsedBones;
  bool Loop;

  void DrawDebugMenu();
private:

};

class SkeletalAnimationSet
{
public:
  SkeletalAnimationSet() : Parent(nullptr), CurrAnimation(-1), CurrentTime(0.0f), CurrFrame(0), Animations(), Playing(false), PlaySpeed(10.f), UseCustomTime(false) {};
  SkeletalAnimationSet(SkinnedMesh* parent, const std::vector<SkeletalAnimation> &animations) : Parent(parent), CurrAnimation(-1), CurrentTime(0.0), CurrFrame(0), Animations(animations), Playing(false), PlaySpeed(10.f), UseCustomTime(false) {};

  bool Playing;
  unsigned int CurrFrame;
  float CurrentTime;
  int CurrAnimation;
  std::vector<SkeletalAnimation> Animations;
  SkinnedMesh* Parent;
  float PlaySpeed;
  bool UseCustomTime;
  float CustomTime;

  void SetAnimation(int newAnim);
  void Start();
  void Resume() { Playing = true; };
  void Stop() { Playing = false; };
  void Reset() { CurrentTime = 0.0f; CurrFrame = 0; }

  void DrawDebugMenu();
private:

};

#endif