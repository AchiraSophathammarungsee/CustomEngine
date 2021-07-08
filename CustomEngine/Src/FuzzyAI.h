#pragma once
#ifndef FUZZYAI_H
#define FUZZYAI_H

#include <glm/glm.hpp>
#include <vector>

class FuzzyAI;

struct FuzzyMember
{
  virtual float Interpolate(float input) = 0;
  virtual void DrawDebugMenu(int id, int offset, FuzzyAI *parent) = 0;
};

struct TriangularFuzzyMember : public FuzzyMember
{
  TriangularFuzzyMember(float start, float mid, float end) : start(start), middle(mid), end(end) {};

  float Interpolate(float input) override;
  void DrawDebugMenu(int id, int offset, FuzzyAI* parent) override;

  float start;
  float middle;
  float end;
};

struct TrapezoidFuzzyMember : public FuzzyMember
{
  TrapezoidFuzzyMember(float low, float high, bool left) : low(low), high(high), left(left) {};

  float Interpolate(float input) override;
  void DrawDebugMenu(int id, int offset, FuzzyAI* parent) override;

  float low;
  float high;
  bool left;
};

class FuzzyAI
{
public:
  FuzzyAI() {};
  FuzzyAI(float start, float end, const glm::vec3* targetPredator, int antecedentsNum, int sampleFreq);
  ~FuzzyAI();

  void AddAntecedents(int id, FuzzyMember* f);
  void AddConsequences(int id, FuzzyMember* f);

  const glm::vec3 *predatorPos;
  std::vector<FuzzyMember*> antecedents;
  std::vector<FuzzyMember*> consequences;
  std::vector<float> alphas;
  glm::vec3 velocity;
  int sampleFrequency;
  float start;
  float end;
  float distance;
  std::vector<float> result;
  float centerOfGravity;
  bool freeze;
  float slowDown;

  void DrawDebugMenu();
private:

};

#endif