#include "ClothComp.h"
#include <imgui.h>

ClothComp::ClothComp(float scale, unsigned int resolution, float mass, float airdrag, float structconst, float shearconst, float flexconst, float dampconst, float colliderRad) :
  Scale(scale), Resolution(resolution), Mass(mass), AirDrag(airdrag), VertexColliderRadius(colliderRad), buffersOnGPU(false), ShowDebug(false),
  primitiveMode(PM_Triangles), VAO(0), VBO(0), EBO(0), DampConstant(dampconst), material(nullptr), Gravity(1.f), Wind(glm::vec3(0.f, 0.f, 1.f)), timer(0.f)
{
  SpringConstant[ClothSpring::ST_Structural] = structconst;
  SpringConstant[ClothSpring::ST_Shear] = shearconst;
  SpringConstant[ClothSpring::ST_Flexion] = flexconst;
}

ClothComp::~ClothComp()
{
  ClearMesh();
}

void ClothComp::Reset()
{
  float gridRatio = 1.f / (float)Resolution;
  glm::vec3 ScaleOffset(-Scale * 0.5f, Scale * 0.5f, 0.f);

  timer = 0.f;

  for (unsigned int y = 0; y <= Resolution; y++)
  {
    for (unsigned int x = 0; x <= Resolution; x++)
    {
      ClothVertex &vert = ClothVertices[y * (Resolution + 1) + x];
      vert.position = Scale * glm::vec3(gridRatio * x, -(gridRatio * y), 0.f) + ScaleOffset;
      vert.prevPos = vert.position;
      vert.velocity = glm::vec3(0.f);
      vert.acceleration = glm::vec3(0.f);
      vert.force = glm::vec3(0.f);
    }
  }

  for (ClothSpring& spring : ClothSprings)
  {
    spring.restLength = glm::distance(ClothVertices[spring.vertexIDs[0]].position, ClothVertices[spring.vertexIDs[1]].position);
  }
}

static void CreateSpring(ClothSpring::SpringTypes type, unsigned int vert0, unsigned int vert1, std::vector<ClothVertex> &ClothVertices, std::vector<ClothSpring> &ClothSprings)
{
  ClothSpring spring;
  spring.type = type;
  spring.vertexIDs[0] = vert0;
  spring.vertexIDs[1] = vert1;

  float len = glm::distance(ClothVertices[spring.vertexIDs[0]].position, ClothVertices[spring.vertexIDs[1]].position);
  spring.restLength = len;
  spring.currLength = len;

  ClothVertices[spring.vertexIDs[0]].springConnections.push_back(SpringConnect(ClothSprings.size(), spring.vertexIDs[1]));
  ClothVertices[spring.vertexIDs[1]].springConnections.push_back(SpringConnect(ClothSprings.size(), spring.vertexIDs[0]));

  ClothSprings.push_back(spring);
}

void ClothComp::GenerateMesh()
{
  float gridRatio = 1.f / (float)Resolution;
  glm::vec3 ScaleOffset(-Scale * 0.5f);

  for (unsigned int y = 0; y <= Resolution; y++)
  {
    for (unsigned int x = 0; x <= Resolution; x++)
    {
      ClothVertex vert;
      vert.id = ClothVertices.size();
      vert.position = Scale * glm::vec3(gridRatio * x, -(gridRatio * y), 0.f);
      vert.prevPos = vert.position;
      vert.velocity = glm::vec3(0.f);
      vert.acceleration = glm::vec3(0.f);
      vert.force = glm::vec3(0.f);
      vert.pin = false;

      ClothVertices.push_back(vert);
    }
  }

  ClothVertices[0].pin = true;
  ClothVertices[Resolution].pin = true;

  // horizontal springs
  for (unsigned int y = 0; y <= Resolution; y++)
  {
    for (unsigned int x = 0; x < Resolution; x++)
    {
      CreateSpring(ClothSpring::ST_Structural, y * (Resolution + 1) + x, y * (Resolution + 1) + x + 1, ClothVertices, ClothSprings);
    }
  }

  // vertical springs
  for (unsigned int x = 0; x <= Resolution; x++)
  {
    for (unsigned int y = 0; y < Resolution; y++)
    {
      CreateSpring(ClothSpring::ST_Structural, y * (Resolution + 1) + x, (y + 1) * (Resolution + 1) + x, ClothVertices, ClothSprings);
    }
  }

  // cross springs
  for (unsigned int y = 0; y < Resolution; y++)
  {
    for (unsigned int x = 0; x < Resolution; x++)
    {
      CreateSpring(ClothSpring::ST_Shear, y * (Resolution + 1) + x, (y + 1) * (Resolution + 1) + x + 1, ClothVertices, ClothSprings);
      CreateSpring(ClothSpring::ST_Shear, (y + 1) * (Resolution + 1) + x, y * (Resolution + 1) + x + 1, ClothVertices, ClothSprings);
    }
  }

  // horizontal flexion springs
  for (unsigned int y = 0; y <= Resolution; y++)
  {
    for (unsigned int x = 0; x < Resolution - 1; x++)
    {
      CreateSpring(ClothSpring::ST_Flexion, y * (Resolution + 1) + x, y * (Resolution + 1) + x + 2, ClothVertices, ClothSprings);
    }
  }

  // vertical flexion springs
  for (unsigned int x = 0; x <= Resolution; x++)
  {
    for (unsigned int y = 0; y < Resolution - 1; y++)
    {
      CreateSpring(ClothSpring::ST_Flexion, y * (Resolution + 1) + x, (y + 2) * (Resolution + 1) + x, ClothVertices, ClothSprings);
    }
  }

  // construct mesh
  unsigned int index = 0;
  for (unsigned int y = 0; y < Resolution; y++)
  {
    for (unsigned int x = 0; x < Resolution; x++)
    {
      ClothVertex& tl = ClothVertices[y * (Resolution + 1) + x];
      ClothVertex& bl = ClothVertices[(y + 1) * (Resolution + 1) + x];
      ClothVertex& tr = ClothVertices[y * (Resolution + 1) + x + 1];
      ClothVertex& br = ClothVertices[(y + 1) * (Resolution + 1) + x + 1];

      // quad
      Vertex vertTL(tl.position, glm::vec3(0.f, 0.f, 1.f));
      Vertex vertBL(bl.position, glm::vec3(0.f, 0.f, 1.f));
      Vertex vertTR(tr.position, glm::vec3(0.f, 0.f, 1.f));
      Vertex vertBR(br.position, glm::vec3(0.f, 0.f, 1.f));

      // upper triangle
      verticesBuffer.push_back(vertTL);
      indicesBuffer.push_back(index);
      tl.meshVertIDs.push_back(index);
      index++;

      verticesBuffer.push_back(vertBL);
      indicesBuffer.push_back(index);
      bl.meshVertIDs.push_back(index);
      index++;

      verticesBuffer.push_back(vertTR);
      indicesBuffer.push_back(index);
      tr.meshVertIDs.push_back(index);
      index++;

      // lower triangle
      verticesBuffer.push_back(vertTR);
      indicesBuffer.push_back(index);
      tr.meshVertIDs.push_back(index);
      index++;

      verticesBuffer.push_back(vertBL);
      indicesBuffer.push_back(index);
      bl.meshVertIDs.push_back(index);
      index++;

      verticesBuffer.push_back(vertBR);
      indicesBuffer.push_back(index);
      br.meshVertIDs.push_back(index);
      index++;
    }
  }
}

void ClothComp::RegisterMesh()
{
  ClearMesh();

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, verticesBuffer.size() * sizeof(Vertex), &verticesBuffer[0], GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesBuffer.size() * sizeof(unsigned int), &indicesBuffer[0], GL_STATIC_DRAW);

  // vertex positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  // vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  // vertex tangent
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
  // vertex bitangent
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
  // vertex texture coords
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

  glBindVertexArray(0);

  buffersOnGPU = true;
}

void ClothComp::UpdateMesh()
{
  for (ClothVertex& clothVert : ClothVertices)
  {
    for (unsigned int vID : clothVert.meshVertIDs)
    {
      verticesBuffer[vID].position = clothVert.position;

      /*for (SpringConnect& connect : clothVert.springConnections)
      {
        verticesBuffer[vID].normal += verticesBuffer[connect.otherVertID].normal;
      }
      verticesBuffer[vID].normal /= clothVert.springConnections.size();*/
    }
  }

  for (unsigned int i = 0; i < indicesBuffer.size(); i += 3)
  {
    glm::vec3 tangent = verticesBuffer[i + 1].position - verticesBuffer[i].position;
    glm::vec3 bitangent = verticesBuffer[i + 2].position - verticesBuffer[i].position;

    glm::vec3 norm = glm::normalize(glm::cross(tangent, bitangent));

    verticesBuffer[i + 0].normal = norm;
    verticesBuffer[i + 1].normal = norm;
    verticesBuffer[i + 2].normal = norm;
  }

  if (buffersOnGPU)
  {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, verticesBuffer.size() * sizeof(Vertex), &verticesBuffer[0]);
  }
}

void ClothComp::ClearMesh()
{
  if (buffersOnGPU)
  {
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VAO);

    buffersOnGPU = false;
  }
}

void ClothComp::Draw()
{
  glBindVertexArray(VAO);
  glDrawElements(primitiveMode, indicesBuffer.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void ClothComp::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("ClothComp"))
  {
    ImGui::Separator();
    ImGui::Indent(10.f);

    ImGui::InputFloat("Cloth Scale", &Scale);
    if (ImGui::Button("Reset")) Reset();

    ImGui::Text("Resolution: %d", Resolution);
    ImGui::Text("Timer: %f", timer);
    ImGui::InputFloat("Mass", &Mass);
    ImGui::InputFloat("Structural Constant", &SpringConstant[ClothSpring::ST_Structural]);
    ImGui::InputFloat("Shear Constant", &SpringConstant[ClothSpring::ST_Shear]);
    ImGui::InputFloat("Flexion Constant", &SpringConstant[ClothSpring::ST_Flexion]);
    ImGui::InputFloat("Damp Constant", &DampConstant);
    ImGui::InputFloat("Air Resistance", &AirDrag);
    ImGui::InputFloat("Gravity", &Gravity);
    ImGui::InputFloat3("Wind Direction", &Wind[0]);
    ImGui::InputFloat("Vertex Collider Radius", &VertexColliderRadius);

    ImGui::Checkbox("Show Spring-Point", &ShowDebug);

    ImGui::Text("Vertices: %d", verticesBuffer.size());
    ImGui::Text("Indices: %d", indicesBuffer.size());

    static const char* primitivemodes[PM_COUNT] = { "Points", "Lines", "Line_Loop", "Line_Strip", "Triangles", "Triangle_Strip", "Triangle_Fan" };
    ImGui::Text("Primitive Mode: %s", primitivemodes[primitiveMode]);

    ImGui::Unindent(10.f);
    ImGui::Separator();
  }
}