/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Model.h
Purpose: Encapsulate everything related to 3D model (load, draw, delete, transform, vertex-calculation, etc.)
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/11/2019
End Header --------------------------------------------------------*/

#include "Model.h"
#include <algorithm>
#include "ModelLoader.h"
#include "UVWrap.h"
#include "Framebuffer.h"
#include "ResourceManager.h"
#include "Renderer.h"

#define PREVIEWRESOLUTION 200

Model::Model(std::vector<Vertex>& _vertices, std::vector<Face>& _faces, PrimitiveMode mode, NormalBaseMode nbm, bool dynamicDraw, bool needPreview) :
  verticesData(_vertices), facesData(_faces), primitiveMode(mode), buffersOnGPU(false), NormalMode(nbm), VAO(0), VBO(0), EBO(0), LineVAO(0), LineVBO(0), noFineData(false), dynamicDraw(dynamicDraw)
{
  CalculateFaceNormals();

  ConstructBuffers(nbm);
  UpdateBufferNormal(nbm);
  RegisterBuffers();

  ConstructNormalLines(0.05f);
}

Model::Model(std::vector<Vertex>& _vertices, std::vector<unsigned int>& _indices, PrimitiveMode mode, bool dynamicDraw, bool needPreview) :
  verticesBuffer(_vertices), indicesBuffer(_indices), primitiveMode(mode), buffersOnGPU(false), NormalMode(NBM_None), VAO(0), VBO(0), EBO(0), LineVAO(0), LineVBO(0), noFineData(true), dynamicDraw(dynamicDraw)
{
  RegisterBuffers();
}

Model::~Model()
{
  ClearBuffers();

  if (noFineData == false)
  {
    glDeleteBuffers(1, &LineVBO);
    glDeleteBuffers(1, &LineVAO);
  }
}

void Model::ClearBuffers()
{
  if (buffersOnGPU)
  {
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VAO);

    buffersOnGPU = false;
  }
}

bool Model::LoadModel(const std::string& filepath)
{
  std::vector<Vertex> vertices;
  std::vector<Face> faces;
  BoundingBox bb;
  if (ReloadModel(filepath.c_str(), vertices, faces, bb))
  {
    verticesData = vertices;
    facesData = faces;
    boundingBox = bb;

    CalculateFaceNormals();

    ConstructBuffers(NormalMode);
    UpdateBufferNormal(NormalMode);
    RegisterBuffers();

    ConstructNormalLines(0.05f);

    Path = filepath;
    return true;
  }
  return false;
}

void Model::UpdateBuffer()
{
  if (buffersOnGPU)
  {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, verticesBuffer.size() * sizeof(Vertex), &verticesBuffer[0]);
  }
}

void Model::Draw()
{
  //Shader* specialShader = material.GetSpecialShader();
  //if (specialShader) shader = specialShader;

  glBindVertexArray(VAO);
  glDrawElements(primitiveMode, indicesBuffer.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void Model::DrawNormal()
{
  if (noFineData == false)
  {
    glBindVertexArray(LineVAO);
    glDrawArrays(PM_Lines, 0, normalLines.size());
    glBindVertexArray(0);
  }
}

void Model::CalculateFaceNormals()
{
  for (Face& f : facesData)
  {
    glm::vec3 edge1 = verticesData[f.indices[1]].position - verticesData[f.indices[0]].position;
    glm::vec3 edge2 = verticesData[f.indices[2]].position - verticesData[f.indices[0]].position;
    f.normal = glm::cross(edge1, edge2);
    f.normal = glm::normalize(f.normal);

    glm::vec2 deltaUV1 = verticesData[f.indices[1]].uv - verticesData[f.indices[0]].uv;
    glm::vec2 deltaUV2 = verticesData[f.indices[2]].uv - verticesData[f.indices[0]].uv;
    float c = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    f.tangent.x = c * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    f.tangent.y = c * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    f.tangent.z = c * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    f.tangent = glm::normalize(f.tangent);

    f.bitangent.x = c * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    f.bitangent.y = c * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    f.bitangent.z = c * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    f.bitangent = glm::normalize(f.bitangent);
  }
}

void Model::ConstructNormalLines(float length)
{
  for (Face& f : facesData)
  {
    int v1 = f.indices[0];
    int v2 = f.indices[1];
    int v3 = f.indices[2];

    float centerX = (verticesData[v1].position.x + verticesData[v2].position.x + verticesData[v3].position.x) / 3.f;
    float centerY = (verticesData[v1].position.y + verticesData[v2].position.y + verticesData[v3].position.y) / 3.f;
    float centerZ = (verticesData[v1].position.z + verticesData[v2].position.z + verticesData[v3].position.z) / 3.f;
    glm::vec3 faceCenter(centerX, centerY, centerZ);

    normalLines.push_back(faceCenter);
    normalLines.push_back(faceCenter + (f.normal * length));
  }

  glGenVertexArrays(1, &LineVAO);
  glGenBuffers(1, &LineVBO);

  glBindVertexArray(LineVAO);
  glBindBuffer(GL_ARRAY_BUFFER, LineVBO);

  glBufferData(GL_ARRAY_BUFFER, normalLines.size() * sizeof(glm::vec3), &normalLines[0], (dynamicDraw) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

  // vertex positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindVertexArray(0);
}

void Model::UpdateBufferNormal(NormalBaseMode nbm)
{
  if (nbm == NBM_Face)
  {
    for (unsigned int i = 0; i < facesData.size(); i++)
    {
      verticesBuffer[i * 3].normal = facesData[i].normal;
      verticesBuffer[i * 3].tangent = facesData[i].tangent;
      verticesBuffer[i * 3].bitangent = facesData[i].bitangent;

      verticesBuffer[i * 3 + 1].normal = facesData[i].normal;
      verticesBuffer[i * 3 + 1].tangent = facesData[i].tangent;
      verticesBuffer[i * 3 + 1].bitangent = facesData[i].bitangent;

      verticesBuffer[i * 3 + 2].normal = facesData[i].normal;
      verticesBuffer[i * 3 + 2].tangent = facesData[i].tangent;
      verticesBuffer[i * 3 + 2].bitangent = facesData[i].bitangent;
    }
  }
  else if(nbm == NBM_Vertex)
  {
    // for each face
    for (Face &f : facesData)
    {
      // for each vertex of the face
      for (int i = 0; i < 3; i++)
      {
        // find faces that also share this vertex
        Vertex& currVert = verticesBuffer[f.indices[i]];

        // if have sharing faces, vertex normal = average of all sharing face normals
        if (!currVert.sharingFaces.empty())
        {
          std::vector<unsigned int> calculatedFaces;
          calculatedFaces.reserve(currVert.sharingFaces.size());

          for (unsigned int shareFace : currVert.sharingFaces)
          {
            // checking for parallel faces with same normals, exclude it
            bool exclude = false;
            for(unsigned int face : calculatedFaces)
            {
              if (facesData[face].normal == facesData[shareFace].normal)
              {
                exclude = true;
                break;
              }
            }

            if (exclude)
              continue;

            currVert.normal += facesData[shareFace].normal;
            currVert.tangent += facesData[shareFace].tangent;
            currVert.bitangent += facesData[shareFace].bitangent;

            calculatedFaces.push_back(shareFace);
          }
          currVert.normal = glm::normalize(currVert.normal);
          currVert.tangent = glm::normalize(currVert.tangent);
          currVert.bitangent = glm::normalize(currVert.bitangent);
        }
        else
        {
          currVert.normal = glm::vec3(0.f);
          currVert.tangent = glm::vec3(0.f);
          currVert.bitangent = glm::vec3(0.f);
        }
      }
    }
  }
  else
  {
    for (Vertex &v : verticesBuffer)
    {
      v.normal = glm::vec3(0.f);
      v.tangent = glm::vec3(0.f);
      v.bitangent = glm::vec3(0.f);
    }
  }
}

void Model::ConstructBuffers(NormalBaseMode nbm)
{
  verticesBuffer.clear();
  indicesBuffer.clear();

  if (nbm == NBM_Face)
  {
    int i = 0;
    for (Face& f : facesData)
    {
      // duplicate any shared vertices
      verticesBuffer.push_back(verticesData[f.indices[0]]);
      verticesBuffer.push_back(verticesData[f.indices[1]]);
      verticesBuffer.push_back(verticesData[f.indices[2]]);

      indicesBuffer.push_back(i);
      i++;
      indicesBuffer.push_back(i);
      i++;
      indicesBuffer.push_back(i);
      i++;
    }
  }
  else if (nbm == NBM_Vertex)
  {
    for (Face& f : facesData)
    {
      indicesBuffer.push_back(f.indices[0]);
      indicesBuffer.push_back(f.indices[1]);
      indicesBuffer.push_back(f.indices[2]);
    }
    verticesBuffer = verticesData;
  }
}

void Model::RegisterBuffers()
{
  ClearBuffers();

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, verticesBuffer.size() * sizeof(Vertex), &verticesBuffer[0], GL_STATIC_DRAW);

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

  //if(preview) GeneratePreview(false, false);
}

void Model::ChangeNormalBase(NormalBaseMode nbm)
{
  if (nbm == NBM_None || NormalMode == NBM_None)
    return;

  if (nbm != NormalMode)
  {
    NormalMode = nbm;
    ConstructBuffers(nbm);
    UpdateBufferNormal(nbm);
    RegisterBuffers();
  }
}

//void Model::SetBoundingBox(BoundingBox& box)
//{ 
//  boundingBox = box;
//  uvWrap.SetBB(box);
//}

void Model::CalculateUVfromUVWrap(void)
{
  uvWrap.CalculateUVontoModel(verticesData);
  ConstructBuffers(NormalMode);
  UpdateBufferNormal(NormalMode);
  RegisterBuffers();
}

void Model::DrawDebugMenu()
{
  ImGui::Text("File: %s", Path.c_str());
  static bool previewing = false;
  //if (ImGui::ImageButton(ImTextureID(preview->GetTexture(0).texture->GetID()), ImVec2(80, 80)))
  //{
  //  previewing = true;
  //}
  //if (previewing)
  //{
  //  ImGui::SetNextWindowSize(ImVec2(310.f, 400.f));
  //  ImGui::Begin(("Model Preview"), &previewing);

  //  static bool showBB = false;
  //  static bool showWireframe = false;
  //  static bool draggingRotate = false;
  //  static bool draggingScale = false;
  //  static glm::vec2 initDragPos;
  //  static float initAngle = 0.f;
  //  static float initScale = 1.f;

  //  ImGui::Checkbox("Show BB", &showBB);
  //  ImGui::SameLine();
  //  ImGui::Checkbox("Show Wireframe", &showWireframe);
  //  ImGui::SameLine();
  //  if (ImGui::Button("Reset View"))
  //  {
  //    previewTransform = Transform();
  //  }
  //  //GeneratePreview(showBB, showWireframe);
  //  //ImGui::Image(ImTextureID(preview->GetTexture(0).texture->GetID()), ImVec2(300, 300));
  //  ImGui::CaptureMouseFromApp();
  //  if (ImGui::IsMouseHoveringWindow())
  //  {
  //    if (ImGui::IsMouseClicked(1))
  //    {
  //      draggingRotate = true;
  //      ImVec2 mousePos = ImGui::GetMousePos();
  //      initDragPos = glm::vec2(mousePos.x, mousePos.y);
  //      initAngle = previewTransform.GetRotationAngle();
  //    }
  //    else if (ImGui::IsMouseClicked(2))
  //    {
  //      draggingScale = true;
  //      ImVec2 mousePos = ImGui::GetMousePos();
  //      initDragPos = glm::vec2(mousePos.x, mousePos.y);
  //      initScale = previewTransform.GetScale().x;
  //    }
  //    if (draggingRotate)
  //    {
  //      ImVec2 mousePos = ImGui::GetMousePos();
  //      float xDelta = mousePos.x - initDragPos.x;
  //      //glm::vec2 mouseDelta(mousePos.x - initDragPos.x, mousePos.y - initDragPos.y);

  //      previewTransform.SetRotationAxis(glm::vec3(0.f, 1.f, 0.f));
  //      float angle = xDelta;
  //      previewTransform.SetRotationAngle(initAngle + angle);

  //      if (ImGui::IsMouseReleased(1))
  //      {
  //        draggingRotate = false;
  //      }
  //    }
  //    else if(draggingScale)
  //    {
  //      ImVec2 mousePos = ImGui::GetMousePos();
  //      float deltaMouse = (mousePos.x - initDragPos.x) / 310.f;
  //      previewTransform.SetScale(glm::vec3(initScale) + glm::vec3(deltaMouse));

  //      if (ImGui::IsMouseReleased(2))
  //      {
  //        draggingScale = false;
  //      }
  //    }
  //  }
  //  else
  //  {
  //    draggingRotate = false;
  //    draggingScale = false;
  //  }

  //  ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Right-click drag: rotate");
  //  ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Middle-click drag: scale");

  //  ImGui::End();
  //}
  /*static char filePath[256];
  if (ImGui::InputText("Load File", filePath, 256))
  {
    LoadModel(filePath);
  }*/
  /*if (ImGui::IsItemHovered())
  {
    ImGui::BeginTooltip();
    ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Reload model from obj file");
    ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "input is a file path relative to the exe.");
    ImGui::EndTooltip();
  }*/

  ImGui::Text("Vertices: %d", verticesData.size());
  ImGui::Text("Faces: %d", facesData.size());
  ImGui::Text("Indices: %d", indicesBuffer.size());

  static const char* primitivemodes[PM_COUNT] = { "Points", "Lines", "Line_Loop", "Line_Strip", "Triangles", "Triangle_Strip", "Triangle_Fan" };
  ImGui::Text("Primitive Mode: %s", primitivemodes[primitiveMode]);

  if (noFineData)
  {
    ImGui::Text("Normal Mode: Vertex");
  }
  else
  {
    static const char* normalmodes[3] = { "None", "Vertex", "Face" };
    static int normalmode = NormalMode;
    if (ImGui::ListBox("Normal Mode", &normalmode, normalmodes, 3))
      ChangeNormalBase(static_cast<NormalBaseMode>(normalmode));
    if (ImGui::IsItemHovered())
    {
      ImGui::BeginTooltip();
      ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Change how vertex normals are calculated.");
      ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Vertex: average normal from all neightbor faces (smooth)");
      ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Face: use normal from the face vertex is a part of (flat)");
      ImGui::EndTooltip();
    }
  }

  //if (boundingVolume.DrawDebugMenu())
  //{
  //  //boundingVolume.UpdateVolume();
  //}

  ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Edit UV mapping here");
  uvWrap.DrawDebugMenu(this);
}

void Model::GeneratePreview(Model* mesh, Framebuffer* preview, Shader* previewShader, Transform *transform)
{
  previewShader->use();
  previewShader->setVec3("color", glm::vec3(0.8f));
  previewShader->setMat4("modelMatrix", transform->GetMatrix());
  previewShader->setInt("disableShade", false);
  preview->BeginCapture();

  ShaderConfig::PolygonMode mode = Renderer::GetPolygonMode();
  Renderer::SetPolygonMode(ShaderConfig::Fill);

  glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  mesh->Draw();

  preview->EndCapture();
  Renderer::SetPolygonMode(mode);
}

void Model::DrawResourceCreator(bool& openFlag)
{
  static char nameBuffer[256] = "unnamed";
  static char pathBuffer[256] = "../Common/models/";
  static bool normalizeScale = true;
  static const char* normalmodes[3] = { "None", "Vertex", "Face" };
  static int normalmode = NormalBaseMode::NBM_Vertex;

  ImGui::SetNextWindowSize(ImVec2(300, 100));
  ImGui::Begin("New mesh", &openFlag);

  ImGui::InputText("Mesh name", nameBuffer, 255);
  ImGui::InputText("Mesh path", pathBuffer, 255);
  ImGui::ListBox("Normal Mode", &normalmode, normalmodes, 3);
  ImGui::Checkbox("Normalize Scale", &normalizeScale);

  if (ImGui::Button("Create Mesh"))
  {
    ResourceManager::Add<Model>(LoadModelFile(pathBuffer, static_cast<NormalBaseMode>(normalmode), normalizeScale), nameBuffer);
    openFlag = false;
  }

  ImGui::End();
}