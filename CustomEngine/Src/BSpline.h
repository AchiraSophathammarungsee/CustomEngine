#pragma once
#ifndef BSPLINE_H
#define BSPLINE_H

#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include "Vertex.h"

class Shader;
class Model;

struct DistanceValue
{
  DistanceValue(float _t, float dist) : t(_t), distance(dist) {};

  float t;
  float distance;
};

class SplineMesh
{
public:
  SplineMesh() : VBO(0), VAO(0), EBO(0), Vertices(), Indices(), OnGPU(false) {};
  ~SplineMesh();

  void RegisterMesh(unsigned int vertexCount);
  void UpdateMesh();
  void Draw();

  std::vector<Vertex>& GetVertices();

private:
  GLuint VBO, VAO, EBO;
  std::vector<Vertex> Vertices;
  std::vector<unsigned int> Indices;
  Model* Mesh;
  bool OnGPU;
};

class BSpline
{
public:
  BSpline() : 
    Resolution(0), Mesh(), GroundMat(), Play(false), TableResolution(100), TotalDistance(0.f), Distance(0.f), Speed(1.f), AdaptiveTolerance(0.1f), Easing(true), Time(0.f), 
    RampUpTime(1.f), RampDownTime(1.f), UseAdaptiveMethod(true), AnimationWalkSpeed(0.1f), EaseDur(30.f)
  {};
  BSpline(unsigned int resolution) : 
    Resolution(resolution), Mesh(), GroundMat(), Play(false), TableResolution(100), TotalDistance(0.f), Distance(0.f), Speed(1.f), AdaptiveTolerance(0.1f), Easing(true), Time(0.f), 
    RampUpTime(1.f), RampDownTime(1.f), UseAdaptiveMethod(true), AnimationWalkSpeed(0.1f), EaseDur(30.f)
  { Init(); };

  void Init();
  void CalculateMesh();
  void Update();
  void Draw(Shader *shader);

  glm::vec3 SampleSpline(float t);
  void AddControlPoint(const glm::vec3& p);
  bool IsPlaying() const { return Play; };
  void IncDistance() { Distance += Speed; if (Distance > TotalDistance) Distance = TotalDistance; };
  float GetDistance() const { return Distance; };
  void Start() { Distance = 0.f; Time = 0.f; Play = true; };
  void Stop() { Play = false; };
  float GetSpeed() const { return Speed; };

  void CreateDistTable();
  void AdaptiveDistTable();
  float SamplingDistance(float distance);
  float GetTotalDist() const { return TotalDistance; };

  bool IsEasing() const { return Easing; };
  void SetEasing(bool flag) { Easing = flag; };

  void DrawDebugMenu();

  bool Play;
  float Speed;
  float Distance;
  bool Easing;
  float Time;
  float RampUpTime;
  float RampDownTime;
  bool UseAdaptiveMethod;
  float AnimationWalkSpeed;
  float AnimSpeed;
  float EaseDur;

private:
  std::vector<glm::vec3> ControlPoints;

  unsigned int Resolution;
  SplineMesh Mesh;

  glm::mat4 GroundMat;

  int TableResolution;
  std::vector<DistanceValue> DistanceTable;
  float TotalDistance;
  float AdaptiveTolerance;
};

#endif