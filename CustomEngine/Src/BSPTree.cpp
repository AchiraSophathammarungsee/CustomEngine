#include "BSPTree.h"
#include "RenderObject.h"
#include "Model.h"
#include <algorithm>
#include "Renderer.h"
#include "EngineCore.h"

#define BSPCOLORCOUNT 8
glm::vec3 bspColorLevels[BSPCOLORCOUNT] = { glm::vec3(1.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 1.f), glm::vec3(1.f, 0.f, 1.f), glm::vec3(0.5f, 0.8f, 0.5f) };

BSPTree::BSPTree() : root(nullptr), drawLevel(-1), terminateTriangles(300), maxDrawLevel(0), iterationLimit(25), leafStoring(true), drawIndex(0), scheme(BSPPS_Balanced_Tree), calcCount(0)
{
  root = new BSPNode(this, 0);
}

BSPTree::~BSPTree()
{
  delete root;
}

BSPNode::~BSPNode()
{
  Clear();
}

void BSPNode::Clear()
{
  if (childs[0])
  {
    delete childs[0];
    childs[0] = nullptr;
  }
  if (childs[1])
  {
    delete childs[1];
    childs[1] = nullptr;
  }

  if (model)
  {
    delete model;
  }
  model = nullptr;
}

void BSPTree::Calculate()
{
  root->Clear();
  maxDrawLevel = 0;
  drawIndex = 0;
  std::vector<BSPPoly> triangles;

  /*const std::vector< RenderObject*> objs = RenderSystem::GetInstance()->GetObjects();

  for (RenderObject* obj : objs)
  {
    Model* mesh = obj->GetModel();
    std::vector<Vertex>& vertices = mesh->GetVertices();
    std::vector<unsigned>& indices = mesh->GetIndices();

    int i = 0;
    BSPPoly poly;
    for (unsigned indice : indices)
    {
      glm::vec3 worldPos = obj->transform.GetMatrix() * glm::vec4(vertices[indice].position, 1.f);
      poly.InsertVertex(i, worldPos);
      i++;
      if (i == 3)
      {
        triangles.push_back(poly);
        i = 0;
      }
    }
  }

  root->Split(triangles, 0);*/
}

float Dot(const glm::vec3& a, const glm::vec3& b)
{
  return a.x* b.x + a.y * b.y + a.z * b.z;
}

bool Plane::IsInside(const glm::vec3& point) const
{
  return Dot(point - position, normal) >= 0.f;
}

glm::vec3 linePlaneIntersect(const glm::vec3& start, const glm::vec3& dir, const Plane& plane)
{
  glm::vec3 diff = plane.position - start;
  float lengthToPlane = Dot(diff, plane.normal) / Dot(dir, plane.normal);
  return start + dir * lengthToPlane;
}

void TrianglePlaneUtil(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const Plane& plane, std::vector<BSPPoly>& positive, std::vector<BSPPoly>& negative)
{
  glm::vec3 lineAC = C - A;
  glm::vec3 lineACDir = glm::normalize(lineAC);
  glm::vec3 intersectAC = linePlaneIntersect(A, lineACDir, plane);

  glm::vec3 lineBC = C - B;
  glm::vec3 lineBCDir = glm::normalize(lineBC);
  glm::vec3 intersectBC = linePlaneIntersect(B, lineBCDir, plane);

  if (plane.IsInside(C))
  {
    BSPPoly posTri;
    posTri.vertices[0] = intersectAC;
    posTri.vertices[1] = intersectBC;
    posTri.vertices[2] = C;
    positive.push_back(posTri);

    BSPPoly negTri1;
    negTri1.vertices[0] = intersectAC;
    negTri1.vertices[1] = intersectBC;
    negTri1.vertices[2] = A;
    negative.push_back(negTri1);

    BSPPoly negTri2;
    negTri2.vertices[0] = A;
    negTri2.vertices[1] = intersectBC;
    negTri2.vertices[2] = B;
    negative.push_back(negTri2);
  }
  else
  {
    BSPPoly negTri;
    negTri.vertices[0] = intersectAC;
    negTri.vertices[1] = intersectBC;
    negTri.vertices[2] = C;
    negative.push_back(negTri);

    BSPPoly posTri1;
    posTri1.vertices[0] = intersectAC;
    posTri1.vertices[1] = intersectBC;
    posTri1.vertices[2] = A;
    positive.push_back(posTri1);

    BSPPoly posTri2;
    posTri2.vertices[0] = A;
    posTri2.vertices[1] = intersectBC;
    posTri2.vertices[2] = B;
    positive.push_back(posTri2);
  }
}

void TrianglePlaneIntersect(const BSPPoly& triangle, const Plane& plane, std::vector<BSPPoly>& positive, std::vector<BSPPoly>& negative)
{
  bool positiveVertices[3] = { false, false, false };

  if(plane.IsInside(triangle.vertices[0])) positiveVertices[0] = true;
  if(plane.IsInside(triangle.vertices[1])) positiveVertices[1] = true;
  if(plane.IsInside(triangle.vertices[2])) positiveVertices[2] = true;

  // trivial rejection
  if (positiveVertices[0] == positiveVertices[1] && positiveVertices[1] == positiveVertices[2])
  {
    if (positiveVertices[0])
    {
      positive.push_back(triangle);
    }
    else
    {
      negative.push_back(triangle);
    }
  }
  else
  {
    // it is strandling between the plane
    // split them
    if (positiveVertices[0] == positiveVertices[1])
      TrianglePlaneUtil(triangle.vertices[0], triangle.vertices[1], triangle.vertices[2], plane, positive, negative);
    else if (positiveVertices[1] == positiveVertices[2])
      TrianglePlaneUtil(triangle.vertices[1], triangle.vertices[2], triangle.vertices[0], plane, positive, negative);
    else
      TrianglePlaneUtil(triangle.vertices[0], triangle.vertices[2], triangle.vertices[1], plane, positive, negative);
  }
}

void SplitTriangles(const std::vector<BSPPoly>& triangles, const Plane& plane, std::vector<BSPPoly>& positive, std::vector<BSPPoly>& negative)
{
  for (const BSPPoly& tri : triangles)
  {
    TrianglePlaneIntersect(tri, plane, positive, negative);
  }
}

void numberOfIntersections(const std::vector<BSPPoly>& triangles, const Plane &plane, unsigned &intersectCount, unsigned &positiveCount, unsigned &negativeCount)
{
  unsigned chooseNum = triangles.size() / 5;
  for (unsigned i = 0; i < chooseNum; i++)
  {
    const BSPPoly& tri = triangles[i * 5];

    bool positiveVertices[3] = { false, false, false };

    if (plane.IsInside(tri.vertices[0])) positiveVertices[0] = true;
    if (plane.IsInside(tri.vertices[1])) positiveVertices[1] = true;
    if (plane.IsInside(tri.vertices[2])) positiveVertices[2] = true;

    if (positiveVertices[0] == positiveVertices[1] && positiveVertices[1] == positiveVertices[2])
    {
      if (positiveVertices[0]) positiveCount++;
      else negativeCount++;
    }
    else
    {
      intersectCount++;
    }
  }
}

Plane BSPNode::ChoosePlane(const std::vector<BSPPoly>& triangles)
{
  Plane outPlane;
  if (parent->scheme == BSPPS_Less_Intersect)
  {
    bool foundPlane = false;
    unsigned minIntersections = std::numeric_limits<unsigned>::max();
    unsigned minDiffFromIdeal = std::numeric_limits<unsigned>::max();
    unsigned chooseNum = triangles.size() / 5;
    
    for (unsigned i = 0; i < chooseNum; i++)
    {
      const BSPPoly& tri = triangles[i * 5];

      Plane samplePlane;
      samplePlane.position = (tri.vertices[0] + tri.vertices[1] + tri.vertices[2]) / 3.f;
      samplePlane.normal = glm::normalize(glm::cross(tri.vertices[1] - tri.vertices[0], tri.vertices[2] - tri.vertices[0]));

      unsigned intersectCount = 0;
      unsigned positiveCount = 0;
      unsigned negativeCount = 0;
      numberOfIntersections(triangles, samplePlane, intersectCount, positiveCount, negativeCount);
      unsigned idealDiffPos = std::abs((int)positiveCount - (int)parent->terminateTriangles);
      unsigned idealDiffNeg = std::abs((int)negativeCount - (int)parent->terminateTriangles);
      unsigned idealDiff = std::min(idealDiffPos, idealDiffNeg);

      if (intersectCount < minIntersections && intersectCount > 1)
      {
        outPlane = samplePlane;
        minIntersections = intersectCount;
        minDiffFromIdeal = idealDiff;
        foundPlane = true;
      }
    }

    if (foundPlane == false)
    {
      glm::vec3 middlePoint(0.f);
      for (const BSPPoly& tri : triangles)
      {
        middlePoint += tri.vertices[0];
      }
      middlePoint /= (float)triangles.size();
      outPlane.position = middlePoint;

      unsigned int minIntersections = std::numeric_limits<unsigned int>::max();
      for (int i = 0; i < 10; i++)
      {
        glm::vec3 normal((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
        Plane samplePlane = outPlane;
        samplePlane.normal = glm::normalize(normal);
        unsigned intersectCount = 0;
        unsigned positiveCount = 0;
        unsigned negativeCount = 0;
        numberOfIntersections(triangles, samplePlane, intersectCount, positiveCount, negativeCount);

        if (intersectCount < minIntersections && intersectCount > 1)
        {
          outPlane = samplePlane;
          minIntersections = intersectCount;
        }
      }
    }
  }
  else
  {
    // try to minimize sizes difference between negative and positive polygons
    glm::vec3 middlePoint(0.f);
    for (const BSPPoly& tri : triangles)
    {
      middlePoint += tri.vertices[0];
    }
    middlePoint /= (float)triangles.size();
    outPlane.position = middlePoint;

    unsigned int minIntersections = std::numeric_limits<unsigned int>::max();
    for (int i = 0; i < 10; i++)
    {
      glm::vec3 normal((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
      Plane samplePlane = outPlane;
      samplePlane.normal = glm::normalize(normal);
      unsigned intersectCount = 0;
      unsigned positiveCount = 0;
      unsigned negativeCount = 0;
      numberOfIntersections(triangles, samplePlane, intersectCount, positiveCount, negativeCount);

      if (intersectCount < minIntersections && intersectCount > 1)
      {
        outPlane = samplePlane;
        minIntersections = intersectCount;
      }
    }
  }

  return outPlane;
}

void BSPNode::Split(const std::vector<BSPPoly> &triangles, unsigned level)
{
  parent->maxDrawLevel = std::max(parent->maxDrawLevel, (int)level);

  if (parent->leafStoring == false)
  {
    polygons = triangles;
    GenerateMesh();
  }

  if (triangles.size() > parent->terminateTriangles && level < parent->iterationLimit)
  {
    splitPlane = ChoosePlane(triangles);

    std::vector<BSPPoly> positiveTri, negativeTri;
    SplitTriangles(triangles, splitPlane, positiveTri, negativeTri);

    if (positiveTri.empty() == false)
    {
      childs[0] = new BSPNode(parent, level + 1);
      childs[0]->Split(positiveTri, level + 1);
    }
    if (negativeTri.empty() == false)
    {
      childs[1] = new BSPNode(parent, level + 1);
      childs[1]->Split(negativeTri, level + 1);
    }
  }
  else
  {
    if (parent->leafStoring)
    {
      polygons = triangles;
      GenerateMesh();
    }
  }
}

void BSPNode::GenerateMesh()
{
  if (polygons.empty() == false && this->model == nullptr)
  {
    drawIndex = parent->drawIndex;
    parent->drawIndex++;

    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    unsigned modelIndices = 0;
    for (BSPPoly& triangle : polygons)
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
}

void BSPTree::Draw()
{
  root->Draw();
}

void BSPNode::Draw()
{
  if (parent->leafStoring)
  {
    if (model) Renderer::AddDebugModel(model, bspColorLevels[drawIndex % BSPCOLORCOUNT]);
    if (childs[0]) childs[0]->Draw();
    if (childs[1]) childs[1]->Draw();
  }
  else
  {
    if (level == parent->drawLevel)
    {
      if (model) Renderer::AddDebugModel(model, bspColorLevels[drawIndex % BSPCOLORCOUNT]);
    }
    else
    {
      if (parent->drawLevel == -1)
      {
        if (model) Renderer::AddDebugModel(model, bspColorLevels[drawIndex % BSPCOLORCOUNT]);
      }

      if (childs[0]) childs[0]->Draw();
      if (childs[1]) childs[1]->Draw();
    }
  }
}

void BSPTree::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("BSP Tree"))
  {
    static int bspterminateT = terminateTriangles;
    if (ImGui::InputInt("BSP Triangles number to Terminate", &bspterminateT))
    {
      bspterminateT = std::max(bspterminateT, 0);
      terminateTriangles = bspterminateT;
    }

    ImGui::SliderInt("BSP Iterations Limit", &iterationLimit, 1, 50);

    ImGui::Text("Draw level:");
    ImGui::Text("[-1] = draw all levels");
    ImGui::Text("[0-max] = specific level");
    ImGui::SliderInt("BSP Draw Level", &drawLevel, -1, maxDrawLevel);

    ImGui::Checkbox("Leaf Storing", &leafStoring);

    static int BSPpartscheme = scheme;
    static const char *BSPpartSchemeStrings[BSPPS_COUNT] = { "Minimize Intersections", "Balanced Tree" };
    if (ImGui::ListBox("Partition Scheme", &BSPpartscheme, BSPpartSchemeStrings, BSPPS_COUNT))
    {
      scheme = static_cast<BSPPartitionScheme>(BSPpartscheme);
    }

    if (ImGui::Button("Calculate BSP Tree"))
    {
      Calculate();
    }
  }
}