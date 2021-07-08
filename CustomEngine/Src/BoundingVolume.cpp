#include "BoundingVolume.h"
#include "Model.h"
#include "Renderer.h"
#include "EngineCore.h"
#include "RenderObject.h"
#include <algorithm>
#include <numeric>

void BoundingVolume::UpdateVolume(const glm::vec3& pos, float _radius)
{
  center = pos;

  if (type == BVT_Sphere)
  {
    radius = _radius;
  }
  else
  {
    min = pos - glm::vec3(_radius);
    max = pos + glm::vec3(_radius);
  }
}

void BoundingVolume::UpdateVolume(const glm::vec3& pos, const std::vector<glm::vec3>& vertices)
{
  center = pos;

  if (type == BVT_Sphere)
  {
    radius = 0.f;
    for (const glm::vec3& v : vertices)
    {
      glm::vec3 diff = v - pos;
      float dist = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

      radius = dist > radius ? dist : radius;
    }
    radius = sqrtf(radius);
  }
  else
  {
    max = glm::vec3(-std::numeric_limits<float>::max());
    min = glm::vec3(std::numeric_limits<float>::max());

    for (const glm::vec3& v : vertices)
    {
      max.x = fmaxf(v.x, max.x);
      max.y = fmaxf(v.y, max.y);
      max.z = fmaxf(v.z, max.z);

      min.x = fminf(v.x, min.x);
      min.y = fminf(v.y, min.y);
      min.z = fminf(v.z, min.z);
    }
  }
}

void BoundingVolume::Draw()
{
  if (type == BVT_AABB)
  {
    Renderer::DebugLineCube(Transform(center, glm::vec3(0.f, 1.f, 0.f), (max - min)), glm::vec3(0.f, 1.f, 0.f));
  }
  else
  {
    Renderer::DebugLineSphere(Transform(center, glm::vec3(0.f, 1.f, 0.f), glm::vec3(radius * 2.f)), glm::vec3(0.f, 1.f, 0.f));
  }
}

bool BoundingVolume::DrawDebugMenu()
{
  bool changedType = false;

  ImGui::Separator();
  ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "BOUNDING VOLUME");
  static const char* typestring[] = { "AABB", "Sphere" };
  static int bvtype = type;
  if (ImGui::ListBox("Bounding Volume Type", &bvtype, typestring, BVT_COUNT))
  {
    type = static_cast<BoundingVolumeTypes>(bvtype);
    changedType = true;
    //ComputeVolume();
  }

  //ImGui::Checkbox("Hide Bounding Volume", &hide);
  ImGui::Separator();

  return changedType;
}

void BoundingVolume::ComputeAABB(std::vector<Vertex>& vertices)
{
  /*glm::vec3 biggestAxises(-std::numeric_limits<float>::max());
  glm::vec3 lowestAxises(std::numeric_limits<float>::max());
  for (Vertex& v : vertices)
  {
    biggestAxises.x = (v.position.x > biggestAxises.x) ? v.position.x : biggestAxises.x;
    biggestAxises.y = (v.position.y > biggestAxises.y) ? v.position.y : biggestAxises.y;
    biggestAxises.z = (v.position.z > biggestAxises.z) ? v.position.z : biggestAxises.z;
    lowestAxises.x = (v.position.x < lowestAxises.x) ? v.position.x : lowestAxises.x;
    lowestAxises.y = (v.position.y < lowestAxises.y) ? v.position.y : lowestAxises.y;
    lowestAxises.z = (v.position.z < lowestAxises.z) ? v.position.z : lowestAxises.z;
  }
  transform.SetScale(biggestAxises - lowestAxises);

  glm::vec3 center = glm::vec3(biggestAxises + lowestAxises) / 2.f;
  transform.SetPosition(center);
  transform.UpdateMatrix();*/
}

void BoundingVolume::ComputeCentroid(std::vector<Vertex>& vertices)
{
  /*glm::vec3 center(0.f);
  for (Vertex& v : vertices)
  {
    center += v.position;
  }
  center /= (float)vertices.size();
  transform.SetPosition(center);

  float biggestDistance = 0.f;
  for (Vertex& v : vertices)
  {
    glm::vec3 diff = center - v.position;
    float distance = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    if (distance > biggestDistance)
      biggestDistance = distance;
  }

  transform.SetScale(glm::vec3(biggestDistance * 2.f));
  transform.UpdateMatrix();*/
}

float distanceBetweenPoints(const glm::vec3& a, const glm::vec3& b)
{
  glm::vec3 diff = a - b;
  return sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
}

glm::vec3 FindLargestDistanceFromPoint(const glm::vec3 &point, const std::vector<Vertex> &vertices)
{
  float biggestDistance = 0.f;
  glm::vec3 farthestPoint(0.f);
  for (const Vertex& v : vertices)
  {
    float distance = distanceBetweenPoints(point, v.position);
    if (distance > biggestDistance)
    {
      biggestDistance = distance;
      farthestPoint = v.position;
    }
  }
  return farthestPoint;
}

bool allPointsInSphere(glm::vec3& center, float &radius, const std::vector<Vertex>& vertices)
{
  for (const Vertex& v : vertices)
  {
    float distance = distanceBetweenPoints(center, v.position);
    if (distance > radius)
    {
      float distOutsideBoundary = distance - radius;
      glm::vec3 towardPointOutside = glm::normalize(v.position - center);
      center = center + towardPointOutside * distOutsideBoundary / 2.f;
      radius = radius + distOutsideBoundary / 2.f;
      return false;
    }
  }
  return true;
}

void BoundingVolume::ComputeRitter()
{
  //Model* model = parent->GetModel();
  //std::vector<Vertex> vertices = model->GetVertices();

  //glm::vec3 px, py, pz;
  //int pickedPoints = 0;
  //for (Vertex& v : vertices)
  //{
  //  glm::vec4 vert = parent->transform.GetMatrix() * glm::vec4(v.position, 1.f);
  //  v.position = glm::vec3(vert.x, vert.y, vert.z);
  //}

  //// pick first point px, then point py farthest from px, then point pz farthest from py
  //px = vertices[0].position;
  //py = FindLargestDistanceFromPoint(px, vertices);
  //pz = FindLargestDistanceFromPoint(py, vertices);

  //glm::vec3 center = (pz + py) / 2.f;
  //float radius = distanceBetweenPoints(pz, py) / 2.f;

  //// find point outside of the sphere, continue to expand until enclose all points
  //while (allPointsInSphere(center, radius, vertices) == false) {}

  //transform.SetPosition(center);
  //transform.SetScale(glm::vec3(radius * 2.f));
  //transform.UpdateMatrix();
}

void BoundingVolume::ComputeLarsson()
{
  /*Model* model = parent->GetModel();
  std::vector<Vertex> vertices = model->GetVertices();

  for (Vertex& v : vertices)
  {
    glm::vec4 vert = parent->transform.GetMatrix() * glm::vec4(v.position, 1.f);
    v.position = glm::vec3(vert.x, vert.y, vert.z);
  }
*/

}

void BoundingVolume::ComputeCovarianceMatrix(std::vector<Vertex> &vertices, glm::mat3 &outMatrix)
{
  //// get average values for each axis
  //glm::vec3 averagePerAxis(0.f);
  //for (Vertex& v : vertices)
  //{
  //  glm::vec4 vert = parent->transform.GetMatrix() * glm::vec4(v.position, 1.f);
  //  v.position = glm::vec3(vert.x, vert.y, vert.z);
  //  averagePerAxis += v.position;
  //}
  //averagePerAxis /= (float)vertices.size();

  //// calculate deviation for each axis
  //std::vector<glm::vec3> deviations;
  //deviations.reserve(vertices.size());
  //for (Vertex& v : vertices)
  //{
  //  deviations.push_back(v.position - averagePerAxis);
  //}

  //// construct covariance matrix
  //for (glm::vec3& dev : deviations)
  //{
  //  outMatrix[0][0] += dev.x * dev.x;
  //  outMatrix[0][1] += dev.x * dev.y;
  //  outMatrix[0][2] += dev.x * dev.z;
  //  outMatrix[1][1] += dev.y * dev.y;
  //  outMatrix[1][2] += dev.y * dev.z;
  //  outMatrix[2][2] += dev.z * dev.z;
  //}
  //outMatrix /= (float)deviations.size();
  //// copy over upper triangle terms since it is diagonally symmetric
  //outMatrix[1][0] = outMatrix[0][1];
  //outMatrix[2][0] = outMatrix[0][2];
  //outMatrix[2][1] = outMatrix[1][2];
}

// 2-by-2 Symmetric Schur decomposition. Given an n-by-n symmetric matrix
// and indices row, column such that 1 <=row<column<=n,computes a sine-cosine pair
// (s, c) that will serve to form a Jacobi rotation matrix.
void SymSchur2(const glm::mat3& covarianceMat, int row, int column, float& c, float& s)
{
  if (std::abs(covarianceMat[row][column]) > 0.0001f)
  {
    float r = (covarianceMat[column][column] - covarianceMat[row][row]) / (2.f * covarianceMat[row][column]);
    float t;
    if (r >= 0.f)
    {
      t = 1.f / (r + sqrt(1.f + r*r));
    }
    else
    {
      t = -1.f / (-r + sqrt(1.f + r*r));
    }
    c = 1.f / sqrt(1.f + t*t);
    s = t * c;
  }
  else
  {
    c = 1.f;
    s = 0.f;
  }
}

void ComputeJacobi(glm::mat3 &covarianceMat, glm::mat3 &eigenVectors)
{
  float prevoffset, c, s;
  glm::mat3 J;// , b, t;

  // Repeat for some maximum number of iterationsconst
  int MAX_ITERATIONS = 50;
  for (int n = 0; n < MAX_ITERATIONS; n++)
  {
    // Find largest off-diagonal absolute element a[p][q
    unsigned row = 0, column = 1;
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        if (i == j) continue;

        if (std::abs(covarianceMat[i][j]) > std::abs(covarianceMat[row][column]))
        {
          row = i;
          column = j;
        }
      }
    }

    // Compute the Jacobi rotation matrix J(p, q, theta)
    SymSchur2(covarianceMat, row, column, c, s);
    for (int i = 0; i < 3; i++)
    {
      J[i][0] = 0.f;
      J[i][1] = 0.f;
      J[i][2] = 0.f;
      J[i][i] = 1.f;
    }
    J[row][row] = c;      J[row][column] = s;
    J[column][row] = -s;  J[column][column] = c;

    // Cumulate rotations into what will contain the eigenvectors
    eigenVectors = eigenVectors * J;

    // Make ’covariance matrix’ more diagonal, until just eigenvalues remain on diagonal
    covarianceMat = (glm::transpose(J) * covarianceMat) * J;

    // Compute "norm" of off-diagonal elements
    float offset = 0.0f;
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        if (i == j) continue;

        offset += covarianceMat[i][j] * covarianceMat[i][j];
      }
    }

    // Stop when norm no longer decreasing
    if(n > 2 && offset >= prevoffset)
      return;

    prevoffset = offset;
  }
}

void BoundingVolume::ComputeSpherePCA()
{
  //Model* model = parent->GetModel();
  //std::vector<Vertex> vertices = model->GetVertices();
  //glm::mat3 covarianceMatrix(0.f);
  //glm::mat3 eigenVectors(1.f);
  //
  //ComputeCovarianceMatrix(vertices, covarianceMatrix);

  //ComputeJacobi(covarianceMatrix, eigenVectors);
  //// eigen values are now stored inside covariance matrix

  //// find component with largest eigen value
  //float diagonalA = std::abs(covarianceMatrix[0][0]);
  //float diagonalB = std::abs(covarianceMatrix[1][1]);
  //float diagonalC = std::abs(covarianceMatrix[2][2]);
  //float biggestValue = diagonalA;
  //int maxComponent = 0;

  //if (diagonalB > biggestValue)
  //{
  //  biggestValue = diagonalB;
  //  maxComponent = 1;
  //}
  //if (diagonalC > biggestValue)
  //{
  //  biggestValue = diagonalC;
  //  maxComponent = 2;
  //}
  //glm::vec3 eigenAxis;
  //eigenAxis.x = eigenVectors[0][maxComponent];
  //eigenAxis.y = eigenVectors[1][maxComponent];
  //eigenAxis.z = eigenVectors[2][maxComponent];

  //// find extreme points along direction eigenAxis
  //glm::vec3 max, min;
  //float maxDist = -std::numeric_limits<float>::max();
  //float minDist = std::numeric_limits<float>::max();
  //for (Vertex& v : vertices)
  //{
  //  // project point onto the line
  //  glm::vec3 diff = v.position - parent->transform.GetPosition();
  //  glm::vec3 line = eigenAxis - parent->transform.GetPosition();
  //  float lineMag = glm::distance(line, parent->transform.GetPosition());
  //  glm::vec3 projLine = glm::dot(diff, line) / (lineMag * lineMag) * line;

  //  bool negative = (glm::dot(projLine, eigenAxis) >= 0) ? false : true;
  //  float dist = glm::distance(parent->transform.GetPosition() + projLine, parent->transform.GetPosition());
  //  if (negative) dist = -dist;

  //  // get most negative point + position point on the line
  //  if (dist > maxDist)
  //  {
  //    maxDist = dist;
  //    max = v.position;
  //  }
  //  else if (dist < minDist)
  //  {
  //    minDist = dist;
  //    min = v.position;
  //  }
  //}

  //float dist = glm::distance(min, max);
  //transform.SetPosition(glm::vec3(min + max) / 2.f);
  //transform.SetScale(glm::vec3(dist));
  //transform.UpdateMatrix();
}

void BoundingVolume::ComputeEllipsoidPCA()
{
  //Model* model = parent->GetModel();
  //std::vector<Vertex> vertices = model->GetVertices();
  //glm::mat3 covarianceMatrix(0.f);
  //glm::mat3 eigenVectors(1.f);

  //ComputeCovarianceMatrix(vertices, covarianceMatrix);

  //ComputeJacobi(covarianceMatrix, eigenVectors);
  //// eigen values are now stored inside covariance matrix

  //// find component with largest eigen value
  //float diagonalA = std::abs(covarianceMatrix[0][0]);
  //float diagonalB = std::abs(covarianceMatrix[1][1]);
  //float diagonalC = std::abs(covarianceMatrix[2][2]);
  //float biggestValue = diagonalA;
  //int maxComponent = 0;

  //if (diagonalB > biggestValue)
  //{
  //  biggestValue = diagonalB;
  //  maxComponent = 1;
  //}
  //if (diagonalC > biggestValue)
  //{
  //  biggestValue = diagonalC;
  //  maxComponent = 2;
  //}
  //glm::vec3 eigenAxis;
  //eigenAxis.x = eigenVectors[0][maxComponent];
  //eigenAxis.y = eigenVectors[1][maxComponent];
  //eigenAxis.z = eigenVectors[2][maxComponent];

  //// find extreme points along direction eigenAxis
  //glm::vec3 max, min;
  //float maxDist = -std::numeric_limits<float>::max();
  //float minDist = std::numeric_limits<float>::max();
  //for (Vertex& v : vertices)
  //{
  //  // project point onto the line
  //  glm::vec3 diff = v.position - parent->transform.GetPosition();
  //  float eigenMag = glm::length(eigenAxis);
  //  glm::vec3 projLine = glm::dot(eigenAxis, diff) / (eigenMag * eigenMag) * eigenAxis;
  //  float dist = glm::length(projLine);
  //  if (glm::dot(diff, eigenAxis) < 0) dist = -dist;

  //  // get most negative point + position point on the line
  //  if (dist > maxDist)
  //  {
  //    maxDist = dist;
  //    max = v.position;
  //  }
  //  if (dist < minDist)
  //  {
  //    minDist = dist;
  //    min = v.position;
  //  }
  //}

  //transform.SetPosition(glm::vec3(min + max) / 2.f);
  //transform.SetScale(glm::vec3(max - min));
  //transform.UpdateMatrix();
}

void BoundingVolume::ComputeOBBPCA()
{

}