#include "SkinnedMesh.h"
#include <imgui.h>
#include "Framebuffer.h"
#include "Renderer.h"
#include "ModelLoader.h"
#include "ResourceManager.h"
#include <iomanip>

SkinnedMesh::SkinnedMesh(std::vector<SkinnedVertex>& _vertices, std::vector<unsigned int>& _indices, std::vector<Bone> &_bones, const glm::mat4& invRootMat, PrimitiveMode _mode, bool _dynamicDraw)
  : verticesBuffer(_vertices), indicesBuffer(_indices), bones(_bones), primitiveMode(_mode), dynamicDraw(_dynamicDraw), buffersOnGPU(false), VAO(0), VBO(0), EBO(0), inverseRoot(invRootMat), 
    useAnimations(false), showBones(false), useIK(false)
{
  RegisterBuffers();

  finalMatrixArray.reserve(bones.size());
  for (int i = 0; i < bones.size(); i++)
  {
    finalMatrixArray.push_back(bones[i].FinalMatrix);
  }
}

SkinnedMesh::~SkinnedMesh()
{
  ClearBuffers();
}

void SkinnedMesh::ClearBuffers()
{
  if (buffersOnGPU)
  {
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VAO);

    buffersOnGPU = false;
  }
}

void SkinnedMesh::UpdateBuffer()
{
  if (buffersOnGPU)
  {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, verticesBuffer.size() * sizeof(SkinnedVertex), &verticesBuffer[0]);
  }
}

void SkinnedMesh::Draw()
{
  glBindVertexArray(VAO);
  glDrawElements(primitiveMode, indicesBuffer.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void SkinnedMesh::RegisterBuffers()
{
  ClearBuffers();

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, verticesBuffer.size() * sizeof(SkinnedVertex), &verticesBuffer[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesBuffer.size() * sizeof(unsigned int), &indicesBuffer[0], GL_STATIC_DRAW);

  // vertex positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)0);
  // vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, normal));
  // vertex tangent
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, tangent));
  // vertex bitangent
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, bitangent));
  // vertex texture coords
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, uv));

  glEnableVertexAttribArray(5);
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, boneIDs));

  glEnableVertexAttribArray(6);
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, weights));

  glBindVertexArray(0);

  buffersOnGPU = true;

  //if(preview) GeneratePreview(false, false);
}

void SkinnedMesh::ForwardKinematic(Bone* bone)
{
  if (bone->Parent == -1)
  {
    bone->TransformMatrix = bone->BindposeMatrix;
  }
  else
  {
    bone->TransformMatrix = bones[bone->Parent].TransformMatrix * bone->BindposeMatrix;
  }

  bone->FinalMatrix = bone->TransformMatrix * bone->OffsetMatrix;

  for (unsigned int child : bone->Childrens)
  {
    ForwardKinematic(&bones[child]);
  }
}

void DisplayMatrix(const glm::mat4 &mat)
{
  ImGui::Text("|%.3f|%.3f|%.3f|%.3f|", mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
  ImGui::Text("|%.3f|%.3f|%.3f|%.3f|", mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
  ImGui::Text("|%.3f|%.3f|%.3f|%.3f|", mat[2][0], mat[2][1], mat[2][2], mat[2][3]);
  ImGui::Text("|%.3f|%.3f|%.3f|%.3f|", mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
}

void DisplayDebugBone(const Bone &currBone, const std::vector<Bone> &bones, unsigned int level)
{
  ImGui::Indent(5.f * level);

  ImGui::Separator();
  ImGui::Text("Name: %s", currBone.Name.c_str());
  ImGui::Text("ID: %d", currBone.ID);
  if (currBone.Parent != -1)
  {
    ImGui::Text("Parent: %d", currBone.Parent);
    ImGui::Text("Parent Name: %s", bones[currBone.Parent].Name.c_str());
  }
  else
  {
    ImGui::Text("Parent: NULL");
  }
  ImGui::Text("Final Matrix:");
  DisplayMatrix(currBone.FinalMatrix);
  ImGui::Text("World Matrix:");
  DisplayMatrix(currBone.TransformMatrix);
  ImGui::Text("Animation Matrix:");
  DisplayMatrix(currBone.AnimationMatrix);
  ImGui::Text("Offset Matrix:");
  DisplayMatrix(currBone.OffsetMatrix);
  ImGui::Text("Bind Pose Matrix:");
  DisplayMatrix(currBone.BindposeMatrix);
  ImGui::Separator();

  ImGui::Unindent(5.f * level);

  for (unsigned int childID : currBone.Childrens)
  {
    DisplayDebugBone(bones[childID], bones, level + 1);
  }
}

void SkinnedMesh::DrawDebugMenu()
{
  ImGui::Text("File: %s", Path.c_str());
  static bool previewing = false;

  ImGui::Text("Vertices: %d", verticesBuffer.size());
  ImGui::Text("Indices: %d", indicesBuffer.size());

  static const char* primitivemodes[PM_COUNT] = { "Points", "Lines", "Line_Loop", "Line_Strip", "Triangles", "Triangle_Strip", "Triangle_Fan" };
  ImGui::Text("Primitive Mode: %s", primitivemodes[primitiveMode]);

  ImGui::Checkbox("Uses animations", &useAnimations);
  if (ImGui::Button("Print bones"))
    PrintBones();

  ImGui::Checkbox("Show bones", &showBones);

  if (ImGui::CollapsingHeader("Bones") && bones.empty() == false)
  {
    DisplayDebugBone(bones[0], bones, 0);
  }
}

void SkinnedMesh::GeneratePreview(SkinnedMesh* mesh, Framebuffer* preview, Shader* previewShader, Transform* transform)
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

void SkinnedMesh::DrawResourceCreator(bool& openFlag)
{
  static char nameBuffer[256] = "unnamed";
  static char pathBuffer[256] = "../Common/models/";

  ImGui::SetNextWindowSize(ImVec2(300, 100));
  ImGui::Begin("New mesh", &openFlag);

  ImGui::InputText("Mesh name", nameBuffer, 255);
  ImGui::InputText("Mesh path", pathBuffer, 255);

  if (ImGui::Button("Create Mesh"))
  {
    ResourceManager::Add<SkinnedMesh>(ReadAssimpFile(pathBuffer), nameBuffer);
    openFlag = false;
  }

  ImGui::End();
}

void DispMatrix(const glm::mat4& mat, const std::string &indent)
{
  std::cout.precision(6);
  std::cout << indent << '|' << std::setw(6) << mat[0][0] << '|' << std::setw(6) << mat[0][1] << '|' << std::setw(6) << mat[0][2] << '|' << std::setw(6) << mat[0][3] << '|' << std::endl;
  std::cout << indent << '|' << std::setw(6) << mat[1][0] << '|' << std::setw(6) << mat[1][1] << '|' << std::setw(6) << mat[1][2] << '|' << std::setw(6) << mat[1][3] << '|' << std::endl;
  std::cout << indent << '|' << std::setw(6) << mat[2][0] << '|' << std::setw(6) << mat[2][1] << '|' << std::setw(6) << mat[2][2] << '|' << std::setw(6) << mat[2][3] << '|' << std::endl;
  std::cout << indent << '|' << std::setw(6) << mat[3][0] << '|' << std::setw(6) << mat[3][1] << '|' << std::setw(6) << mat[3][2] << '|' << std::setw(6) << mat[3][3] << '|' << std::endl;
  std::cout.precision();
}

void RecurBone(const Bone& currBone, const std::vector<Bone>& bones, unsigned int level)
{
  std::string indent;
  for (unsigned int i = 0; i < level; i++)
  {
    indent += '.';
  }

  std::cout << indent << "Name: " << currBone.Name << std::endl;
  std::cout << indent << "ID: " << currBone.ID << std::endl;
  if (currBone.Parent != -1)
  {
    std::cout << indent << "Parent: " << bones[currBone.Parent].Name << std::endl;
    std::cout << indent << "Parent ID: " << currBone.Parent << std::endl;
  }
  else
  {
    std::cout << indent << "Parent: NULL" << std::endl;
  }
  std::cout << indent << "Final Matrix:" << std::endl;
  DispMatrix(currBone.FinalMatrix, indent);
  std::cout << indent << "World Matrix:" << std::endl;
  DispMatrix(currBone.TransformMatrix, indent);
  std::cout << indent << "Animation Matrix:" << std::endl;
  DispMatrix(currBone.AnimationMatrix, indent);
  std::cout << indent << "Offset Matrix:" << std::endl;
  DispMatrix(currBone.OffsetMatrix, indent);
  std::cout << indent << "Bind Pose Matrix:" << std::endl;
  DispMatrix(currBone.BindposeMatrix, indent);
  std::cout << "--------------------------------------------" << std::endl;

  for (unsigned int childID : currBone.Childrens)
  {
    RecurBone(bones[childID], bones, level + 1);
  }
}

void SkinnedMesh::PrintBones()
{
  RecurBone(bones[0], bones, 0);
}