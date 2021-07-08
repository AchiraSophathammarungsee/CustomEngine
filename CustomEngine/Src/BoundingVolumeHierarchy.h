#ifndef BOUNDINGVOLUMEHIERARCHY_H
#define BOUNDINGVOLUMEHIERARCHY_H
#include <vector>
#include "BoundingVolume.h"

enum BVHMethod { BVH_Topdown, BVH_Bottomup, BVH_COUNT };

class RenderObject;

struct HierarchyNode
{
public:
  HierarchyNode(BoundingVolumeTypes type, HierarchyNode* parent, HierarchyNode* child1, HierarchyNode* child2, const glm::vec3 &color = glm::vec3(1.f, 1.f, 0.f)) : parent(parent), height(-1)
  {
    childs[0] = child1;
    childs[1] = child2;
    BV = BoundingVolume();
  }
  ~HierarchyNode()
  {
  }

  void Draw();
  void Draw(int height, int n);
  void Clear();

  std::vector<Vertex> vertices;
  HierarchyNode* parent;
  HierarchyNode* childs[2];
  BoundingVolume BV;
  int height;
};

class BoundingVolumeHierarchy
{
public:
  BoundingVolumeHierarchy(BVHMethod method, BoundingVolumeTypes type = BVT_AABB);
  ~BoundingVolumeHierarchy();

  void SetMethod(BVHMethod method);
  void SetType(BoundingVolumeTypes type);
  void AddTreeNode(BoundingVolume bv);
  void RemoveTreeNode(BoundingVolume bv);
  void ComputeTree();
  void ComputeTreeTopDown();
  void ComputeTreeBottomUp();
  void ClearTree();

  void Draw();
  void DrawDebugMenu();

private:
  void TreeDownRecursive(const std::vector<RenderObject*>& objects, const std::vector<std::vector<Vertex>>& objectsVertices, std::vector<unsigned>& indices, int startIndex, int endIndex, HierarchyNode** currNode, int debugHeight);

  BVHMethod method;
  BoundingVolumeTypes type;
  HierarchyNode* root;
  int treeHeight;
  int drawHeight;
  bool drawAll;
};

#endif