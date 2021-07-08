#include "BSpline.h"
#include "shader.hpp"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Model.h"
#include <imgui.h>
#include <math.h>
#include <utility>
#include <stack>

SplineMesh::~SplineMesh()
{
  /*if (OnGPU)
  {
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VAO);
  }*/
}

void SplineMesh::RegisterMesh(unsigned int vertexCount)
{
  for (unsigned int i = 0; i < vertexCount; i++)
  {
    Vertices.push_back(glm::vec3());
    Indices.push_back(i);
  }

  Mesh = new Model(Vertices, Indices, PM_Line_Strip);
  ResourceManager::Add<Model>(Mesh, "BSpline mesh");

  //glGenVertexArrays(1, &VAO);
  //glGenBuffers(1, &VBO);
  //glGenBuffers(1, &EBO);

  //glBindVertexArray(VAO);
  //glBindBuffer(GL_ARRAY_BUFFER, VBO);

  //glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(glm::vec3), &Vertices[0], GL_STATIC_DRAW);

  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  //glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(unsigned int), &Indices[0], GL_STATIC_DRAW);

  //// vertex positions
  //glEnableVertexAttribArray(0);
  //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

  //glBindVertexArray(0);

  //OnGPU = true;
}

void SplineMesh::UpdateMesh()
{
  Mesh->UpdateBuffer();
  /*glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, Vertices.size() * sizeof(glm::vec3), &Vertices[0]);*/
}

void SplineMesh::Draw()
{
  Mesh->Draw();
  /*glBindVertexArray(VAO);
  glDrawElements(GL_LINE_STRIP, Indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);*/
}

std::vector<Vertex>& SplineMesh::GetVertices()
{
  return Mesh->GetVertices();
}

void BSpline::Init()
{
  Mesh.RegisterMesh(Resolution + 1);

  // need at least 4 control points
  AddControlPoint(glm::vec3(2.f, 0.f, 0.f));
  AddControlPoint(glm::vec3(0.f, 0.f, 2.f));
  AddControlPoint(glm::vec3(-2.f, 0.f, 0.f));
  AddControlPoint(glm::vec3(0.f, 0.f, -2.f));
  AddControlPoint(glm::vec3(2.f, 0.f, 0.f));
  AddControlPoint(glm::vec3(0.f, 0.f, 2.f));
  AddControlPoint(glm::vec3(-2.f, 0.f, 0.f));

  // initialize GroundMat values
  GroundMat[0][0] = -1.f;
  GroundMat[0][1] = 3.f;
  GroundMat[0][2] = -3.f;
  GroundMat[0][3] = 1.f;

  GroundMat[1][0] = 3.f;
  GroundMat[1][1] = -6.f;
  GroundMat[1][2] = 3.f;
  GroundMat[1][3] = 0.f;

  GroundMat[2][0] = -3.f;
  GroundMat[2][1] = 0.f;
  GroundMat[2][2] = 3.f;
  GroundMat[2][3] = 0.f;

  GroundMat[3][0] = 1.f;
  GroundMat[3][1] = 4.f;
  GroundMat[3][2] = 1.f;
  GroundMat[3][3] = 0.f;

  GroundMat *= (1.f / 6.f); // constant 1/6

  GroundMat = glm::transpose(GroundMat);

  Update();
}

void BSpline::CalculateMesh()
{
  std::vector<Vertex>& vertices = Mesh.GetVertices();

  int loopNum = ControlPoints.size() - 3;
  if (loopNum == 0) return;

  int resPerLoop = Resolution / loopNum;
  int remainRes = Resolution - (resPerLoop * loopNum);
  for (int i = 0; i < loopNum; i++)
  {
    glm::mat4 pointsMat;
    pointsMat[0] = glm::vec4(ControlPoints[i + 0], 1.f);
    pointsMat[1] = glm::vec4(ControlPoints[i + 1], 1.f);
    pointsMat[2] = glm::vec4(ControlPoints[i + 2], 1.f);
    pointsMat[3] = glm::vec4(ControlPoints[i + 3], 1.f);
    pointsMat = glm::transpose(pointsMat);

    for (int j = 0; j < resPerLoop; j++)
    {
      double step = (float)j / (float)resPerLoop;

      glm::vec4 time;
      time.x = step * step * step;
      time.y = step * step;
      time.z = step;
      time.w = 1.f;

      vertices[resPerLoop * i + j].position = time * GroundMat * pointsMat;
    }
  }

  // remaining line meshes to start point (closed spline)
  for (int i = 0; i < remainRes; i++)
  {
    vertices[resPerLoop * loopNum + i].position = vertices[0].position;
  }
  vertices[Resolution].position = vertices[0].position;
}

void BSpline::Update()
{
  CalculateMesh();
  Mesh.UpdateMesh();
  if (UseAdaptiveMethod)
  {
    AdaptiveDistTable();
  }
  else
  {
    CreateDistTable();
  }
}

void BSpline::Draw(Shader* shader)
{
  shader->use();
  shader->setMat4("modelMatrix", glm::mat4(1.f));
  shader->setVec3("color", glm::vec3(0.f, 1.f, 1.f));
  Mesh.Draw();

  int i = 0;
  for (const glm::vec3& point : ControlPoints)
  {
    Renderer::DebugLineSphere(Transform(point, glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.1f)), glm::vec3(1.f, 1.f, 0.f));

    if (i < ControlPoints.size() - 1)
    {
      Renderer::DebugLine(ControlPoints[i], ControlPoints[i + 1], 1.0F, glm::vec3(1.f, 1.f, 0.f));
    }
    i++;
  }
}

void BSpline::AddControlPoint(const glm::vec3& p)
{
  ControlPoints.push_back(p);
  Update();
}

glm::vec3 BSpline::SampleSpline(float t)
{
  if (t >= 1.f) t = 0.f;

  int loopNum = ControlPoints.size() - 3;

  if (loopNum == 0) return ControlPoints[0];

  int i = floor(t * loopNum);
  float u = (t * loopNum) - i;

  glm::mat4 pointsMat;
  pointsMat[0] = glm::vec4(ControlPoints[i + 0], 1.f);
  pointsMat[1] = glm::vec4(ControlPoints[i + 1], 1.f);
  pointsMat[2] = glm::vec4(ControlPoints[i + 2], 1.f);
  pointsMat[3] = glm::vec4(ControlPoints[i + 3], 1.f);
  pointsMat = glm::transpose(pointsMat);

  glm::vec4 time;
  time.x = u * u * u;
  time.y = u * u;
  time.z = u;
  time.w = 1.f;

  glm::vec4 result = time * GroundMat * pointsMat;
  return glm::vec3(result.x, result.y, result.z);
}

void BSpline::CreateDistTable()
{
  if (ControlPoints.size() < 4) return;

  DistanceTable.clear();

  DistanceTable.push_back(DistanceValue(0.f, 0.f));

  for (int i = 1; i <= TableResolution; i++)
  {
    float prevT = (float)(i - 1) / (float)TableResolution;
    float currT = (float)i / (float)TableResolution;

    float dist = glm::length(SampleSpline(currT) - SampleSpline(prevT));
    DistanceTable.push_back(DistanceValue(currT, DistanceTable[i - 1].distance + dist));
  }

  TotalDistance = DistanceTable.back().distance;
}

void BSpline::AdaptiveDistTable()
{
  if (ControlPoints.size() < 4) return;

  DistanceTable.clear();
  DistanceTable.push_back(DistanceValue(0.f, 0.f));

  std::stack<std::pair<float, float>> UnprocessSegments;
  UnprocessSegments.push(std::pair<float, float>(0.f, 1.f));

  while (UnprocessSegments.empty() == false)
  {
    std::pair<float, float> seg = UnprocessSegments.top();
    UnprocessSegments.pop();

    float midT = (seg.second + seg.first) / 2.f;
    glm::vec3 p0 = SampleSpline(seg.first);
    glm::vec3 p1 = SampleSpline(midT);
    glm::vec3 p2 = SampleSpline(seg.second);

    float distA = glm::length(p1 - p0);
    float distB = glm::length(p2 - p1);
    float distC = glm::length(p2 - p0);
    
    float diff = distA + distB - distC;
    if (diff <= AdaptiveTolerance)
    {
      float prevDist = DistanceTable.back().distance;

      DistanceTable.push_back(DistanceValue(midT, prevDist + distA));
      DistanceTable.push_back(DistanceValue(seg.second, prevDist + distA + distB));
    }
    else
    {
      UnprocessSegments.push(std::pair<float, float>(midT, seg.second));
      UnprocessSegments.push(std::pair<float, float>(seg.first, midT));
    }
  }

  TotalDistance = DistanceTable.back().distance;
}

float BSpline::SamplingDistance(float distance)
{
  if (distance == 0.f) return 0.f;

  int i = 0;
  for (; i <= TableResolution; i++)
  {
    if(distance <= DistanceTable[i].distance) break;
  }

  float a = (distance - DistanceTable[i - 1].distance) / (DistanceTable[i].distance - DistanceTable[i - 1].distance);

  float t = DistanceTable[i - 1].t + a * (DistanceTable[i].t - DistanceTable[i - 1].t);
  return t;
}

void BSpline::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("B Spline"))
  {
    ImGui::Indent(5.f);

    ImGui::Text("Line resolution = %d", Resolution);

    if (ImGui::CollapsingHeader("Control points"))
    {
      int i = 0;
      for (glm::vec3& p : ControlPoints)
      {
        std::string str = "Point[";
        str += std::to_string(i) + ']';
        if (ImGui::InputFloat3(str.c_str(), &p[0]))
        {
          Update();
        }
        i++;
      }
    }
    if (ImGui::Button("Add control point"))
    {
      AddControlPoint(glm::vec3());
    }

    if (Play == false)
    {
      if (ImGui::Button("Play"))
      {
        Start();
      }
    }
    else
    {
      if (ImGui::Button("Stop"))
      {
        Stop();
      }
    }
    ImGui::SliderFloat("Move Speed", &Speed, 0.01f, 10.f);
    ImGui::InputFloat("Ease Duration", &EaseDur);
    ImGui::Text("AnimSpeed: %f", AnimSpeed);

    ImGui::InputInt("Table Resolution", &TableResolution);
    ImGui::Checkbox("Use adaptive method", &UseAdaptiveMethod);
    ImGui::InputFloat("Adaptive tolerance", &AdaptiveTolerance);
    ImGui::Checkbox("Easing In/Out", &Easing);
    ImGui::InputFloat("Ramp up time", &RampUpTime);
    ImGui::InputFloat("Ramp down time", &RampDownTime);
    ImGui::InputFloat("Animation walk speed", &AnimationWalkSpeed);

    ImGui::Text("Current time: %f", Time);
    ImGui::Text("Current distance: %f", Distance);
    ImGui::Text("Total distance: %f", TotalDistance);
    if (ImGui::CollapsingHeader("Distance table"))
    {
      ImGui::Indent(5.f);

      ImGui::Text("|   s    | lengths");
      for (const DistanceValue& val : DistanceTable)
      {
        ImGui::Text("[%f] %f", val.t, val.distance);
      }

      ImGui::Unindent(5.f);
    }

    ImGui::Unindent(5.f);
  }
}