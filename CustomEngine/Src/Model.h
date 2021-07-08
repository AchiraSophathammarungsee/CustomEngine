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

#ifndef MODEL_H
#define MODEL_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <glad/glad.h>
#include "shader.hpp"
#include "Transform.h"
#include "Material.h"
#include <string>
#include "Vertex.h"
#include "BoundingBox.h"
#include "UVWrap.h"
#include "MeshEnum.h"
#include "BoundingVolume.h"

struct Face
{
  Face(glm::vec3 indices = glm::vec3(0), glm::vec3 normal = glm::vec3(0.f)) : indices(indices), normal(normal) {};

  glm::vec3 indices;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 bitangent;
};

class Framebuffer;

class Model
{
public:
  Model(std::vector<Vertex>& _vertices, std::vector<Face>& _faces, PrimitiveMode mode, NormalBaseMode nbm = NBM_Face, bool dynamicDraw = false, bool needPreview = false);
  Model(std::vector<Vertex>& _vertices, std::vector<unsigned int>& _indices, PrimitiveMode mode, bool dynamicDraw = false, bool needPreview = false);
  ~Model();

  void UpdateBuffer();
  std::vector<Vertex>& GetVertices() { return verticesBuffer; };
  std::vector<unsigned>& GetIndices() { return indicesBuffer; };

  void Draw();
  void DrawNormal();

  void ConstructNormalLines(float length);
  
  NormalBaseMode GetNormalBase() { return NormalMode; };
  void ChangeNormalBase(NormalBaseMode nbm);

  void SetFilePath(const std::string& path) { Path = path; };
  const std::string& GetFilePath(void) { return Path; };
  bool LoadModel(const std::string& filepath);

  void CalculateUVfromUVWrap();
  /*void SetUVWrap(UVWrap *uv) { delete uvwrap; uvwrap = new UVWrap(*uv); };
  const UVWrap* GetUVWrap(void) { return uvwrap; };*/

  unsigned int GetVAO() const { return VAO; };
  unsigned int GetIndiceCount() const { return indicesBuffer.size(); };

  void DrawDebugMenu();

  bool haveFineData() { return !noFineData; };

  const std::vector<Vertex>& GetVertexData() const { return verticesData; };
  const std::vector<Face>& GetFaceData() const { return facesData; };
  PrimitiveMode GetPrimitiveMode() const { return primitiveMode; };

  static void GeneratePreview(Model* mesh, Framebuffer* preview, Shader* previewShader, Transform *transform);
  static void DrawResourceCreator(bool& openFlag);

  UVWrap uvWrap;
  BoundingBox boundingBox;
private:
  void UpdateBufferNormal(NormalBaseMode nbm);
  void ConstructBuffers(NormalBaseMode nbm);
  void CalculateFaceNormals();
  void ClearBuffers();
  void RegisterBuffers();
  
  std::string Path;

  // actual vertices and indices buffer for rendering / sending to GPU
  std::vector<Vertex> verticesBuffer;
  std::vector<unsigned int> indicesBuffer;
  unsigned int VAO, VBO, EBO;
  PrimitiveMode primitiveMode;
  bool buffersOnGPU;
  NormalBaseMode NormalMode;

  // storage of pure vertices and faces data
  std::vector<Vertex> verticesData;
  std::vector<Face> facesData;

  std::vector<glm::vec3> normalLines;
  unsigned int LineVAO, LineVBO;

  bool noFineData;
  bool dynamicDraw;
};

#endif //MODEL_H
