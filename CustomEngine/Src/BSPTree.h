#ifndef BSPTREE_H
#define BSPTREE_H

#include <vector>
#include <glm/glm.hpp>
#include "Transform.h"

enum BSPPartitionScheme { BSPPS_Less_Intersect, BSPPS_Balanced_Tree, BSPPS_COUNT };

struct BSPPoly
{
public:
  BSPPoly(const std::vector<glm::vec3>& vertices = std::vector<glm::vec3>(3)) : vertices(vertices) {};
  ~BSPPoly() {};

  void AddVertex(const glm::vec3 vertex) { vertices.push_back(vertex); };
  void InsertVertex(unsigned indice, const glm::vec3 vertex) { vertices[indice] = vertex; };
  std::vector<glm::vec3> vertices;

private:
};

struct Plane
{
public:
  Plane(const glm::vec3& position = glm::vec3(), const glm::vec3& normal = glm::vec3()) : position(position), normal(normal) {};
  ~Plane() {};

  bool IsInside(const glm::vec3 &point) const;

  glm::vec3 position, normal;
  Transform transform;
private:

};

class BSPTree;
class Model;

struct BSPNode
{
public:
  BSPNode(BSPTree* parent, unsigned level) : parent(parent), model(nullptr), level(level), drawIndex(0) { childs[0] = nullptr; childs[1] = nullptr; };
  ~BSPNode();

  void Split(const std::vector<BSPPoly> &triangles, unsigned level);
  void GenerateMesh();
  void Clear();
  void Draw();

  BSPNode* childs[2];
  unsigned level;
  Plane splitPlane;

private:
  Plane ChoosePlane(const std::vector<BSPPoly>& triangles);

  std::vector<BSPPoly> polygons;
  BSPTree* parent;
  Model* model;
  unsigned drawIndex;
};

class RenderObject;

class BSPTree
{
public:
  BSPTree();
  ~BSPTree();

  void Draw();
  void Calculate();

  void DrawDebugMenu();

  int maxDrawLevel;
  unsigned terminateTriangles;
  bool leafStoring;
  int iterationLimit;
  unsigned drawIndex;
  int drawLevel;
  BSPPartitionScheme scheme;
private:
  
  BSPNode* root;
  unsigned calcCount;
  
};

#endif