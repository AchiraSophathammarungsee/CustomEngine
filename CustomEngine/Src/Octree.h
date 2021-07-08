#ifndef OCTREE_H
#define OCTREE_H

#include <vector>
#include <glm/glm.hpp>
#include "Transform.h"
class RenderObject;
class Model;

struct OctPoly
{
public:
  OctPoly(const std::vector<glm::vec3>& vertices = std::vector<glm::vec3>(3)) : vertices(vertices) {};
  ~OctPoly() {};

  void Draw(int level);

  void AddVertex(const glm::vec3 vertex) { vertices.push_back(vertex); };
  void InsertVertex(unsigned indice, const glm::vec3 vertex) { vertices[indice] = vertex; };
  std::vector<glm::vec3> vertices;

private:
  static Transform transform;
};

struct OctNode
{
public:
  OctNode(unsigned level, OctNode *parent, const glm::vec3 &pos, float scale, unsigned index);
  ~OctNode();

  void Draw(int level);

  void AddPoly(const OctPoly &poly);
  void Split(unsigned terminateTriangles, int &maxDrawLevel, int limit);
  void GenerateMesh();

  void Clear();

  glm::vec3 position;
  float scale;
  std::vector<OctPoly> polygons;
  unsigned level;

  OctNode* parent;
  OctNode* childs[8];
  Transform transform;
private:
  Model* model;
  unsigned index;
};

class Octree
{
public:
  Octree();
  ~Octree() { delete root; };

  void Calculate(std::vector<RenderObject*> &objs);
  void Draw();
  void GenerateMeshes();

  void DrawDebugMenu();
private:
  int drawLevel;
  int maxDrawLevel;
  int iterationLimit;
  unsigned terminateTriangles;
  OctNode* root;
};

#endif