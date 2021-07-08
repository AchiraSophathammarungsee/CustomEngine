#include "BoundingVolumeHierarchy.h"
#include "RenderObject.h"
#include "Renderer.h"
#include <algorithm>
#include "Transform.h"
#include "Model.h"
#include "EngineCore.h"

static const glm::vec3 debugColor[7] = {glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f, 1.f, 0.f), glm::vec3(1.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 1.f), glm::vec3(1.f)};

BoundingVolumeHierarchy::BoundingVolumeHierarchy(BVHMethod method, BoundingVolumeTypes type) : method(method), type(type), root(nullptr), drawAll(false), drawHeight(0), treeHeight(0)
{
}

BoundingVolumeHierarchy::~BoundingVolumeHierarchy()
{
  ClearTree();
}

void BoundingVolumeHierarchy::SetMethod(BVHMethod _method)
{
  method = _method;
  ComputeTree();
}

void BoundingVolumeHierarchy::SetType(BoundingVolumeTypes _type)
{
  // we will not use ellipsoid or oriented box
  //if (type != BVT_AABB && type != BVT_SphereCentroid) _type = BVT_AABB;

  type = _type;
  ComputeTree();
}

void ComputeAABB(RenderObject *a, RenderObject*b, glm::vec3 &center, glm::vec3 &scale)
{
  /*Model* modelA = a->GetModel();
  Model* modelB = b->GetModel();
  std::vector<Vertex> verticesA = modelA->GetVertices();
  std::vector<Vertex> verticesB = modelB->GetVertices();

  glm::vec3 biggestAxises(-std::numeric_limits<float>::max());
  glm::vec3 lowestAxises(std::numeric_limits<float>::max());
  for (Vertex& v : verticesA)
  {
    glm::vec4 vert = a->transform.GetMatrix() * glm::vec4(v.position, 1.f);
    glm::vec3 diff = glm::vec3(vert.x, vert.y, vert.z);
    biggestAxises.x = (diff.x > biggestAxises.x) ? diff.x : biggestAxises.x;
    biggestAxises.y = (diff.y > biggestAxises.y) ? diff.y : biggestAxises.y;
    biggestAxises.z = (diff.z > biggestAxises.z) ? diff.z : biggestAxises.z;
    lowestAxises.x = (diff.x < lowestAxises.x) ? diff.x : lowestAxises.x;
    lowestAxises.y = (diff.y < lowestAxises.y) ? diff.y : lowestAxises.y;
    lowestAxises.z = (diff.z < lowestAxises.z) ? diff.z : lowestAxises.z;
  }
  for (Vertex& v : verticesB)
  {
    glm::vec4 vert = b->transform.GetMatrix() * glm::vec4(v.position, 1.f);
    glm::vec3 diff = glm::vec3(vert.x, vert.y, vert.z);
    biggestAxises.x = (diff.x > biggestAxises.x) ? diff.x : biggestAxises.x;
    biggestAxises.y = (diff.y > biggestAxises.y) ? diff.y : biggestAxises.y;
    biggestAxises.z = (diff.z > biggestAxises.z) ? diff.z : biggestAxises.z;
    lowestAxises.x = (diff.x < lowestAxises.x) ? diff.x : lowestAxises.x;
    lowestAxises.y = (diff.y < lowestAxises.y) ? diff.y : lowestAxises.y;
    lowestAxises.z = (diff.z < lowestAxises.z) ? diff.z : lowestAxises.z;
  }
  scale = biggestAxises - lowestAxises;
  center = glm::vec3(biggestAxises + lowestAxises) / 2.f;*/
}

void GetCorners(BoundingVolume *a, BoundingVolume* b, glm::vec3 &outMin, glm::vec3 &outMax)
{
  /*outMin = glm::vec3(std::numeric_limits<float>::max());
  outMax = glm::vec3(-std::numeric_limits<float>::max());

  glm::vec3 lowerCorner = a->GetPosition() - a->GetScale() / 2.f;
  glm::vec3 upperCorner = a->GetPosition() + a->GetScale() / 2.f;
  outMax.x = std::max(outMax.x, upperCorner.x);
  outMax.y = std::max(outMax.y, upperCorner.y);
  outMax.z = std::max(outMax.z, upperCorner.z);
  outMin.x = std::min(outMin.x, lowerCorner.x);
  outMin.y = std::min(outMin.y, lowerCorner.y);
  outMin.z = std::min(outMin.z, lowerCorner.z);

  lowerCorner = b->GetPosition() - b->GetScale() / 2.f;
  upperCorner = b->GetPosition() + b->GetScale() / 2.f;
  outMax.x = std::max(outMax.x, upperCorner.x);
  outMax.y = std::max(outMax.y, upperCorner.y);
  outMax.z = std::max(outMax.z, upperCorner.z);
  outMin.x = std::min(outMin.x, lowerCorner.x);
  outMin.y = std::min(outMin.y, lowerCorner.y);
  outMin.z = std::min(outMin.z, lowerCorner.z);*/
}

void GetSphere(BoundingVolume* a, BoundingVolume* b, glm::vec3 &center, float &diameter)
{
  /*glm::vec3 diff = glm::normalize(a->GetPosition() - b->GetPosition());
  glm::vec3 aEdge = a->GetPosition() + diff * a->GetScale() / 2.f;
  glm::vec3 bEdge = b->GetPosition() - diff * b->GetScale() / 2.f;
  center = (aEdge + bEdge) / 2.f;
  diameter = glm::distance(aEdge, bEdge);*/
}

void PartitionObjects()
{

}

void SortX(const std::vector<RenderObject*>& objects, std::vector<unsigned>& indices, unsigned startIndex, unsigned endIndex)
{
  
}

void BoundingVolumeHierarchy::TreeDownRecursive(const std::vector<RenderObject*>& objects, const std::vector<std::vector<Vertex>> &objectsVertices, std::vector<unsigned> &indices, int startIndex, int endIndex, HierarchyNode **currNode, int debugHeight)
{
  //if (startIndex == endIndex)
  //  return;

  //*currNode = new HierarchyNode(type, nullptr, nullptr, nullptr, debugColor[debugHeight]);
  //for (int i = startIndex; i < endIndex; i++)
  //{
  //  (*currNode)->vertices.insert((*currNode)->vertices.end(), objectsVertices[indices[i]].begin(), objectsVertices[indices[i]].end());
  //}

  //if(type == BVT_AABB)
  //  (*currNode)->BV->ComputeAABB((*currNode)->vertices);
  //else
  //  (*currNode)->BV->ComputeCentroid((*currNode)->vertices);

  //// if only 1 object left, this is a node, return
  //if (endIndex - startIndex == 1)
  //{
  //  if (debugHeight < treeHeight)
  //  {
  //    HierarchyNode* child0 = nullptr;
  //    TreeDownRecursive(objects, objectsVertices, indices, startIndex, endIndex, &child0, debugHeight + 1);
  //    if (child0)
  //      child0->parent = *currNode;
  //    (*currNode)->childs[0] = child0;
  //  }

  //  return;
  //}
  //// else, call recursively on both half of divided objects
  //else
  //{
  //  // partition objects base on axis
  //  // find largest axis to divide
  //  glm::vec3 scale = (*currNode)->BV->GetScale();
  //  int axis = 0;
  //  if (scale.x > scale.y)
  //  {
  //    if (scale.x > scale.z)
  //      axis = 0;
  //    else
  //      axis = 2;
  //  }
  //  else
  //  {
  //    if (scale.y > scale.z)
  //      axis = 1;
  //    else
  //      axis = 2;
  //  }

  //  // sort objects according to the axis (use object's center)
  //  for (int i = startIndex; i < endIndex; i++)
  //  {
  //    for (int j = startIndex; j < endIndex; j++)
  //    {
  //      if (i == j) continue;

  //      if (objects[indices[i]]->GetBoundingVolume()->GetPosition()[axis] > objects[indices[j]]->GetBoundingVolume()->GetPosition()[axis])
  //      {
  //        unsigned tmp = indices[j];
  //        indices[j] = indices[i];
  //        indices[i] = tmp;
  //      }
  //    }
  //  }

  //  // divide objects
  //  int pivot = (startIndex + endIndex) / 2;
  //  HierarchyNode* child0 = nullptr; 
  //  HierarchyNode* child1 = nullptr; 
  //  TreeDownRecursive(objects, objectsVertices, indices, startIndex, pivot, &child0, debugHeight + 1);
  //  TreeDownRecursive(objects, objectsVertices, indices, pivot, endIndex, &child1, debugHeight + 1);

  //  if (child0)
  //    child0->parent = *currNode;
  //  if (child1)
  //    child1->parent = *currNode;

  //  (*currNode)->childs[0] = child0;
  //  (*currNode)->childs[1] = child1;
  //}
}

void BoundingVolumeHierarchy::ComputeTreeTopDown()
{
  /*std::vector<RenderObject*>& objects = Renderer::GetObjects();
  unsigned objectSize = Renderer::GetObjectsSize();
  int actualObjSize = 0;
  std::vector<std::vector<Vertex>> objectsVertices;
  std::vector<unsigned> indices;
  std::vector<RenderObject*> actualObjects;
  for (unsigned i = 0; i < objectSize; i++)
  {
    if (objects[i])
    {
      objects[i]->GetBoundingVolume()->SetType(type);

      std::vector<Vertex> vertices = objects[i]->GetModel()->GetVertices();
      for (Vertex& vert : vertices)
      {
        glm::vec4 worldCoord = objects[i]->transform.GetMatrix() * glm::vec4(vert.position.x, vert.position.y, vert.position.z, 1.f);
        vert = glm::vec3(worldCoord.x, worldCoord.y, worldCoord.z);
      }
      objectsVertices.push_back(vertices);

      indices.push_back(actualObjSize);
      actualObjSize++;
      actualObjects.push_back(objects[i]);
    }
  }
  treeHeight = actualObjSize / 2;

  TreeDownRecursive(actualObjects, objectsVertices, indices, 0, actualObjSize, &root, 0);*/
}

void BoundingVolumeHierarchy::ComputeTreeBottomUp()
{
  //std::vector<RenderObject*>& objects = RenderSystem::GetInstance()->GetObjects();
  //unsigned objectSize = RenderSystem::GetInstance()->GetObjectsSize();

  //std::vector<HierarchyNode*> startingNodes;

  //// create leaf nodes
  //int actualObjSize = 0;
  //for (unsigned i = 0; i < objectSize; i++)
  //{
  //  if (objects[i])
  //  {
  //    objects[i]->GetBoundingVolume()->SetType(type);

  //    HierarchyNode* leafNode = new HierarchyNode(type, nullptr, nullptr, nullptr, debugColor[0]);
  //    leafNode->BV->SetTransform(objects[i]->GetBoundingVolume()->GetTransform());

  //    leafNode->vertices = objects[i]->GetModel()->GetVertices();
  //    for (Vertex& vert : leafNode->vertices)
  //    {
  //      glm::vec4 worldCoord = objects[i]->transform.GetMatrix() * glm::vec4(vert.position.x, vert.position.y, vert.position.z, 1.f);
  //      vert = glm::vec3(worldCoord.x, worldCoord.y, worldCoord.z);
  //    }

  //    startingNodes.push_back(leafNode);

  //    actualObjSize++;
  //  }
  //}
  //treeHeight = actualObjSize / 2;

  //int debugHeight = 1;
  //std::vector<HierarchyNode*> remainingNodes;
  //while (startingNodes.size() > 1)
  //{
  //  
  //  // pair 2 objects with smallest distance together
  //  for (HierarchyNode* a : startingNodes)
  //  {
  //    // skip nodes the already have parent
  //    if (a->parent != nullptr) continue;

  //    HierarchyNode * pair = nullptr;
  //    float smallestVolume = std::numeric_limits<float>::max();
  //    for (HierarchyNode* b : startingNodes)
  //    {
  //      if (b->parent == nullptr && a != b)
  //      {
  //        //float dist = glm::distance(a->BV->GetPosition(), b->BV->GetPosition());

  //        glm::vec3 lowerCorner, upperCorner;
  //        GetCorners(a->BV, b->BV, lowerCorner, upperCorner);
  //        glm::vec3 diff = upperCorner - lowerCorner;
  //        float volume = diff.x * diff.y * diff.z;

  //        if (volume < smallestVolume)
  //        {
  //          pair = b;
  //          smallestVolume = volume;
  //        }
  //      }
  //    }

  //    if (pair)
  //    {
  //      HierarchyNode* node = new HierarchyNode(type, nullptr, a, pair, debugColor[debugHeight]);
  //      node->vertices.insert(node->vertices.begin(), a->vertices.begin(), a->vertices.end());
  //      node->vertices.insert(node->vertices.end(), pair->vertices.begin(), pair->vertices.end());

  //      if (type == BVT_AABB)
  //      {
  //        glm::vec3 lowerCorner, upperCorner;
  //        GetCorners(a->BV, pair->BV, lowerCorner, upperCorner);
  //        node->BV->SetTransform(Transform((lowerCorner + upperCorner) / 2.f, glm::vec3(0.f, 1.f, 0.f), upperCorner - lowerCorner));
  //      }
  //      else
  //      {
  //        node->BV->ComputeCentroid(node->vertices);
  //      }

  //      a->parent = node;
  //      pair->parent = node;

  //      remainingNodes.push_back(node);
  //    }
  //    else
  //    {
  //      HierarchyNode* node = new HierarchyNode(type, nullptr, a, nullptr, debugColor[debugHeight]);
  //      node->BV->SetTransform(a->BV->GetTransform());

  //      node->vertices = a->vertices;

  //      remainingNodes.push_back(node);
  //    }
  //  }
  //  startingNodes.clear();
  //  startingNodes = remainingNodes;
  //  remainingNodes.clear();

  //  debugHeight = (debugHeight > 6) ? 0 : debugHeight + 1;
  //}
  //root = startingNodes[0];
}

void BoundingVolumeHierarchy::ComputeTree()
{
  if (method == BVH_Bottomup)
  {
    ComputeTreeBottomUp();
  }
  else
  {
    ComputeTreeTopDown();
  }
}

void HierarchyNode::Clear()
{
  if (childs[0])
  {
    childs[0]->Clear();
    delete childs[0];
  }
  if (childs[1])
  {
    childs[1]->Clear();
    delete childs[1];
  }
}

void BoundingVolumeHierarchy::ClearTree()
{
  if (root == nullptr) return;

  root->Clear();
  delete root;
  root = nullptr;
  treeHeight = 0;
  drawHeight = 0;
}

void HierarchyNode::Draw(int height, int n)
{
  /*if (height == n)
    BV->Draw();
  else
  {
    if (childs[0]) childs[0]->Draw(height, n+1);
    if (childs[1]) childs[1]->Draw(height, n+1);
  }*/
}

void HierarchyNode::Draw()
{
  if (childs[0]) childs[0]->Draw();
  if (childs[1]) childs[1]->Draw();

  //BV->Draw();
}

void BoundingVolumeHierarchy::Draw()
{
  if (root)
  {
    if (drawAll)
    {
      root->Draw();
    }
    else
    {
      root->Draw(drawHeight, 0);
    }
  }
}

void BoundingVolumeHierarchy::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("Bounding Volume Hierarchy"))
  {
    static const char* methodtring[] = { "TopDown","BottomUp" };
    static int bvhMethod = method;
    if (ImGui::ListBox("Hierachy Method", &bvhMethod, methodtring, BVH_COUNT))
    {
      method = static_cast<BVHMethod>(bvhMethod);
      ComputeTree();
    }

    static const char* typestring[] = { "AABB","Sphere" };
    static int bvtype = type;
    if (ImGui::ListBox("Bounding Volume Type", &bvtype, typestring, 2))
    {
      type = static_cast<BoundingVolumeTypes>(bvtype);
      ComputeTree();
    }

    ImGui::SliderInt("Draw Height", &drawHeight, 0, treeHeight);
    ImGui::Checkbox("Draw All", &drawAll);
    if (ImGui::Button("Calculate Tree"))
      ComputeTree();
    if (ImGui::Button("Clear Tree"))
    {
      ClearTree();
    }
  }
}