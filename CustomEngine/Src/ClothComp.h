#pragma once
#ifndef CLOTHCOMP_H
#define CLOTHCOMP_H

#include <vector>
#include "Vertex.h"
#include "MeshEnum.h"

struct SpringConnect
{
  SpringConnect(unsigned int _springID, unsigned int _otherVertID) : springID(_springID), otherVertID(_otherVertID) {};

  unsigned int springID;
  unsigned int otherVertID;
};

struct ClothVertex
{
  unsigned int id;
  glm::vec3 position;
  glm::vec3 prevPos;
  glm::vec3 velocity;
  glm::vec3 acceleration;
  glm::vec3 force;
  bool pin;

  std::vector<SpringConnect> springConnections;
  std::vector<unsigned int> meshVertIDs;
};

struct ClothSpring
{
  enum SpringTypes
  {
    ST_Structural, ST_Shear, ST_Flexion, ST_COUNT
  };

  SpringTypes type;
  float restLength;
  float currLength;

  // start - end point
  unsigned int vertexIDs[2];
};

class Material;

class ClothComp
{
public:
  ClothComp(float scale = 1.f, unsigned int resolution = 4, float mass = 1.f, float airdrag = 0.2f, float structconst = 0.5f, float shearconst = 0.5f, float flexconst = 0.1f, float dampconst = 0.5f, float colliderRad = 2.f);
  ~ClothComp();

  void GenerateMesh();
  void RegisterMesh();
  void UpdateMesh();
  void ClearMesh();

  void Reset();

  void Draw();

  float GetScale() const { return Scale; };
  unsigned int GetResolution() const { return Resolution; };
  unsigned int GetVAO() const { return VAO; };
  unsigned int GetIndiceCount() const { return indicesBuffer.size(); };
  PrimitiveMode GetPrimitiveMode() const { return primitiveMode; };

  void DrawDebugMenu();

  std::vector<ClothVertex> ClothVertices;
  std::vector<ClothSpring> ClothSprings;
  float AirDrag;
  float DampConstant;
  float SpringConstant[ClothSpring::ST_COUNT];
  // for cloth-cloth collision
  float VertexColliderRadius;
  float Mass;
  float Gravity;
  glm::vec3 Wind;
  float timer;

  Material* material;
  bool ShowDebug;

private:
  // mesh
  std::vector<Vertex> verticesBuffer;
  std::vector<unsigned int> indicesBuffer;
  unsigned int VAO, VBO, EBO;
  PrimitiveMode primitiveMode;
  bool buffersOnGPU;

  float Scale;
  // num of grid per side
  unsigned int Resolution;
};

#endif