#include "Octree.h"
#include "RenderObject.h"
#include "Model.h"
#include "Renderer.h"
#include <algorithm>
#include "EngineCore.h"

#define OCTCOLORCOUNT 8
glm::vec3 colorLevels[OCTCOLORCOUNT] = { glm::vec3(1.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 1.f), glm::vec3(1.f, 0.f, 1.f), glm::vec3(0.5f, 0.8f, 0.5f) };
Transform OctPoly::transform = Transform(glm::vec3(), glm::vec3(), glm::vec3(1.f));

Octree::Octree() : drawLevel(-1), terminateTriangles(300), maxDrawLevel(0), iterationLimit(50)
{
  root = new OctNode(0, nullptr, glm::vec3(), 0.f, 0);
}

void Octree::Calculate(std::vector<RenderObject*>& objs)
{
  //root->Clear();
  //root->scale = 0.f;
  //root->position = glm::vec3();
  //maxDrawLevel = 0;

  //for (RenderObject* obj : objs)
  //{
  //  Model *mesh = obj->GetModel();
  //  std::vector<Vertex> &vertices = mesh->GetVertices();
  //  std::vector<unsigned> &indices = mesh->GetIndices();

  //  BoundingBox box = mesh->GetBoundingBox();

  //  int i = 0;
  //  OctPoly poly;
  //  for (unsigned indice : indices)
  //  {
  //    glm::vec3 worldPos = obj->transform.GetMatrix() * glm::vec4(vertices[indice].position, 1.f);
  //    root->scale = std::max(root->scale, std::abs(worldPos.x));
  //    root->scale = std::max(root->scale, std::abs(worldPos.y));
  //    root->scale = std::max(root->scale, std::abs(worldPos.z));

  //    poly.InsertVertex(i, worldPos);
  //    i++;
  //    if (i == 3)
  //    {
  //      root->AddPoly(poly);
  //      i = 0;
  //    }
  //  }
  //}
  //root->scale *= 2.f; // cube mesh is [-0.5, 0.5]
  //root->transform.SetScale(glm::vec3(root->scale));
  //root->Split(terminateTriangles, maxDrawLevel, iterationLimit);
}

void Octree::Draw()
{
  root->Draw(drawLevel);
}

OctNode::OctNode(unsigned level, OctNode* parent, const glm::vec3& pos, float scale, unsigned index)
  : level(level), parent(parent), position(pos), scale(scale), model(nullptr), index(index)
{
  childs[0] = nullptr;
  childs[1] = nullptr;
  childs[2] = nullptr;
  childs[3] = nullptr;
  childs[4] = nullptr;
  childs[5] = nullptr;
  childs[6] = nullptr;
  childs[7] = nullptr;

  transform.SetPosition(position);
  transform.SetScale(glm::vec3(scale));
  transform.UpdateMatrix();
}

OctNode::~OctNode()
{
  Clear();
}

void OctNode::Draw(int drawLevel)
{
  if (level == drawLevel)
  {
    unsigned colorIndex = level % OCTCOLORCOUNT;
    Renderer::DebugLineCube(transform, colorLevels[colorIndex]);
    if(model) Renderer::AddDebugModel(model, colorLevels[index]);
  }
  else
  {
    if (drawLevel == -1)
    {
      unsigned colorIndex = level % OCTCOLORCOUNT;
      Renderer::DebugLineCube(transform, colorLevels[colorIndex]);
      if (model) Renderer::AddDebugModel(model, colorLevels[colorIndex]);
    }

    if (childs[0])
    {
      childs[0]->Draw(drawLevel);
      childs[1]->Draw(drawLevel);
      childs[2]->Draw(drawLevel);
      childs[3]->Draw(drawLevel);
      childs[4]->Draw(drawLevel);
      childs[5]->Draw(drawLevel);
      childs[6]->Draw(drawLevel);
      childs[7]->Draw(drawLevel);
    }
  }
}

void OctPoly::Draw(int level)
{
  Renderer::DebugLineTriangle(OctPoly::transform, vertices, colorLevels[level % OCTCOLORCOUNT]);
}

void OctNode::AddPoly(const OctPoly& poly)
{
  polygons.push_back(poly);
}

struct planeBits
{
public:
  planeBits() : bits(0) {};
  ~planeBits() {};

  bool operator==(const planeBits &rhs)
  {
    return bits == rhs.bits;
  }

  bool operator[](unsigned index)
  {
    return (1 << index) & bits;
  }

  bool GetIndex(unsigned index)
  {
    return (1 << index) & bits;
  }

  void SetIndex(unsigned index)
  {
    bits = (1 << index) | bits;
  }

  unsigned GetOctIndex()
  {
    unsigned index = 0;
    if (GetIndex(0))
    {
      if (GetIndex(1))
      {
        if (GetIndex(2))
        {
          index = 0;
        }
        else
        {
          index = 4;
        }
      }
      else
      {
        if (GetIndex(2))
        {
          index = 1;
        }
        else
        {
          index = 5;
        }
      }
    }
    else
    {
      if (GetIndex(1))
      {
        if (GetIndex(2))
        {
          index = 2;
        }
        else
        {
          index = 6;
        }
      }
      else
      {
        if (GetIndex(2))
        {
          index = 3;
        }
        else
        {
          index = 7;
        }
      }
    }
    return index;
  }

private:
  unsigned char bits;
};

glm::vec3 linePlaneIntersect(const glm::vec3 &start, const glm::vec3 &dir, int axis, const glm::vec3 &planePos)
{
  float distToPlane = planePos[axis] - start[axis];
  float lengthToPlane = distToPlane / dir[axis];
  return start + dir * lengthToPlane;
}

void TrianglePlaneUtil(const glm::vec3 &A, const glm::vec3& B, const glm::vec3& C, int axis, const glm::vec3& planePos, std::vector<OctPoly>& negative, std::vector<OctPoly>& positive)
{
  glm::vec3 lineAC = C - A;
  glm::vec3 lineACDir = glm::normalize(lineAC);
  glm::vec3 intersectAC = linePlaneIntersect(A, lineACDir, axis, planePos);

  glm::vec3 lineBC = C - B;
  glm::vec3 lineBCDir = glm::normalize(lineBC);
  glm::vec3 intersectBC = linePlaneIntersect(B, lineBCDir, axis, planePos);

  if (C[axis] >= planePos[axis])
  {
    OctPoly posTri;
    posTri.vertices[0] = intersectAC;
    posTri.vertices[1] = intersectBC;
    posTri.vertices[2] = C;
    positive.push_back(posTri);

    OctPoly negTri1;
    negTri1.vertices[0] = intersectAC;
    negTri1.vertices[1] = intersectBC;
    negTri1.vertices[2] = A;
    negative.push_back(negTri1);

    OctPoly negTri2;
    negTri2.vertices[0] = A;
    negTri2.vertices[1] = intersectBC;
    negTri2.vertices[2] = B;
    negative.push_back(negTri2);
  }
  else
  {
    OctPoly negTri;
    negTri.vertices[0] = intersectAC;
    negTri.vertices[1] = intersectBC;
    negTri.vertices[2] = C;
    negative.push_back(negTri);

    OctPoly posTri1;
    posTri1.vertices[0] = intersectAC;
    posTri1.vertices[1] = intersectBC;
    posTri1.vertices[2] = A;
    positive.push_back(posTri1);

    OctPoly posTri2;
    posTri2.vertices[0] = A;
    posTri2.vertices[1] = intersectBC;
    posTri2.vertices[2] = B;
    positive.push_back(posTri2);
  }
}

void TrianglePlaneIntersect(const OctPoly &trianglePoly, int axis, const glm::vec3& planePos, std::vector<OctPoly> &negative, std::vector<OctPoly>& positive)
{
  bool positiveVertices[3] = { false, false, false };

  if (trianglePoly.vertices[0][axis] >= planePos[axis]) positiveVertices[0] = true;
  if (trianglePoly.vertices[1][axis] >= planePos[axis]) positiveVertices[1] = true;
  if (trianglePoly.vertices[2][axis] >= planePos[axis]) positiveVertices[2] = true;

  // trivial rejection
  if (positiveVertices[0] == positiveVertices[1] && positiveVertices[1] == positiveVertices[2])
  {
    if (positiveVertices[0])
    {
      positive.push_back(trianglePoly);
    }
    else
    {
      negative.push_back(trianglePoly);
    }
  }
  else
  {
    // it is strandling between the plane
    // split them
    if (positiveVertices[0] == positiveVertices[1]) 
      TrianglePlaneUtil(trianglePoly.vertices[0], trianglePoly.vertices[1], trianglePoly.vertices[2], axis, planePos, negative, positive);
    else if (positiveVertices[1] == positiveVertices[2]) 
      TrianglePlaneUtil(trianglePoly.vertices[1], trianglePoly.vertices[2], trianglePoly.vertices[0], axis, planePos, negative, positive);
    else 
      TrianglePlaneUtil(trianglePoly.vertices[0], trianglePoly.vertices[2], trianglePoly.vertices[1], axis, planePos, negative, positive);
  }
}

void SplitTriangles(const std::vector<OctPoly> &triangles, int axis, const glm::vec3& planePos, std::vector<OctPoly>& negative, std::vector<OctPoly>& positive)
{
  for (const OctPoly& tri : triangles)
  {
    TrianglePlaneIntersect(tri, axis, planePos, negative, positive);
  }
}

void OctNode::Split(unsigned terminateTriangles, int &maxDrawLevel, int limit)
{
  maxDrawLevel = std::max(maxDrawLevel, (int)level);

  if (polygons.size() > terminateTriangles && level < limit)
  {
    float quarterScale = scale / 4.f;

    childs[0] = new OctNode(level + 1, this, position + glm::vec3(quarterScale, quarterScale, quarterScale), scale / 2.f, 0);
    childs[1] = new OctNode(level + 1, this, position + glm::vec3(quarterScale, -quarterScale, quarterScale), scale / 2.f, 1);
    childs[2] = new OctNode(level + 1, this, position + glm::vec3(-quarterScale, quarterScale, quarterScale), scale / 2.f, 2);
    childs[3] = new OctNode(level + 1, this, position + glm::vec3(-quarterScale, -quarterScale, quarterScale), scale / 2.f, 3);
    childs[4] = new OctNode(level + 1, this, position + glm::vec3(quarterScale, quarterScale, -quarterScale), scale / 2.f, 4);
    childs[5] = new OctNode(level + 1, this, position + glm::vec3(quarterScale, -quarterScale, -quarterScale), scale / 2.f, 5);
    childs[6] = new OctNode(level + 1, this, position + glm::vec3(-quarterScale, quarterScale, -quarterScale), scale / 2.f, 6);
    childs[7] = new OctNode(level + 1, this, position + glm::vec3(-quarterScale, -quarterScale, -quarterScale), scale / 2.f, 7);

    for (OctPoly& triangle : polygons)
    {
      // get each vertex belong to which node
      planeBits vertexBits[3];
      if (triangle.vertices[0].x >= position.x) vertexBits[0].SetIndex(0);
      if (triangle.vertices[0].y >= position.y) vertexBits[0].SetIndex(1);
      if (triangle.vertices[0].z >= position.z) vertexBits[0].SetIndex(2);

      if (triangle.vertices[1].x >= position.x) vertexBits[1].SetIndex(0);
      if (triangle.vertices[1].y >= position.y) vertexBits[1].SetIndex(1);
      if (triangle.vertices[1].z >= position.z) vertexBits[1].SetIndex(2);

      if (triangle.vertices[2].x >= position.x) vertexBits[2].SetIndex(0);
      if (triangle.vertices[2].y >= position.y) vertexBits[2].SetIndex(1);
      if (triangle.vertices[2].z >= position.z) vertexBits[2].SetIndex(2);

      // if triangle is completely contained in a single node,
      if (vertexBits[0] == vertexBits[1] && vertexBits[1] == vertexBits[2])
      {
        unsigned index = vertexBits[0].GetOctIndex();
        childs[index]->AddPoly(triangle);
      }
      else
      {
        // if triangle is stradling between multiple nodes,
        // split polygon base on this node's xyz planes

        // first split on xy-plane
        std::vector<OctPoly> initTriangle;
        initTriangle.push_back(triangle);
        std::vector<OctPoly> posX, negX;
        SplitTriangles(initTriangle, 0, position, negX, posX);

        // split on zy-plane;
        std::vector<OctPoly> posXposY, posXnegY;
        SplitTriangles(posX, 1, position, posXnegY, posXposY);
        std::vector<OctPoly> negXposY, negXnegY;
        SplitTriangles(negX, 1, position, negXnegY, negXposY);

        // split on xz-plane;
        SplitTriangles(posXposY, 2, position, childs[4]->polygons, childs[0]->polygons);
        SplitTriangles(posXnegY, 2, position, childs[5]->polygons, childs[1]->polygons);
        SplitTriangles(negXposY, 2, position, childs[6]->polygons, childs[2]->polygons);
        SplitTriangles(negXnegY, 2, position, childs[7]->polygons, childs[3]->polygons);
      }
    }

    for (int i = 0; i < 8; i++)
    {
      childs[i]->Split(terminateTriangles, maxDrawLevel, limit);
    }
  }
}

void OctNode::Clear()
{
  if (childs[0])
  {
    delete childs[0];
    delete childs[1];
    delete childs[2];
    delete childs[3];
    delete childs[4];
    delete childs[5];
    delete childs[6];
    delete childs[7];

    childs[0] = nullptr;
    childs[1] = nullptr;
    childs[2] = nullptr;
    childs[3] = nullptr;
    childs[4] = nullptr;
    childs[5] = nullptr;
    childs[6] = nullptr;
    childs[7] = nullptr;
  }

  polygons.clear();
  if(model) delete model;
  model = nullptr;
}

void OctNode::GenerateMesh()
{
  if (polygons.empty() == false && this->model == nullptr)
  {
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    unsigned modelIndices = 0;
    for (OctPoly& triangle : polygons)
    {
      vertices.push_back(Vertex(triangle.vertices[0]));
      vertices.push_back(Vertex(triangle.vertices[1]));
      vertices.push_back(Vertex(triangle.vertices[2]));
      indices.push_back(modelIndices);
      modelIndices++;
      indices.push_back(modelIndices);
      modelIndices++;
      indices.push_back(modelIndices);
      modelIndices++;
    }
    this->model = new Model(vertices, indices, PM_Triangles, false, false);
  }

  if (childs[0])
  {
    childs[0]->GenerateMesh();
    childs[1]->GenerateMesh();
    childs[2]->GenerateMesh();
    childs[3]->GenerateMesh();
    childs[4]->GenerateMesh();
    childs[5]->GenerateMesh();
    childs[6]->GenerateMesh();
    childs[7]->GenerateMesh();
  }
}

void Octree::GenerateMeshes()
{
  root->GenerateMesh();
}

void Octree::DrawDebugMenu()
{
  if(ImGui::CollapsingHeader("Octree"))
  {
    static int terminateT = terminateTriangles;
    if(ImGui::InputInt("Triangles number to Terminate", &terminateT))
    {
      terminateT = std::max(terminateT, 0);
      terminateTriangles = terminateT;
    }

    ImGui::SliderInt("Iterations Limit", &iterationLimit, 1, 50);

    ImGui::Text("Draw level:");
    ImGui::Text("[-1] = draw all levels");
    ImGui::Text("[0-max] = specific level");
    ImGui::SliderInt("Draw Level", &drawLevel, -1, maxDrawLevel);

    if (ImGui::Button("Calculate Octree"))
    {
      //Calculate(Renderer::GetObjects());
      //GenerateMeshes();
    }
  }
}