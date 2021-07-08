/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: UVWrap.h
Purpose: Encapsulate all uv wrapping behaviours
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_2
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 11/10/2019
End Header --------------------------------------------------------*/

#ifndef UVWRAP_H
#define UVWRAP_H

#include <glm/glm.hpp>
#include <vector>
#include "Vertex.h"
#include "BoundingBox.h"

enum UVTypes
{
  UVT_Vertex,
  UVT_Planar,
  UVT_Sphere,
  UVT_Cylinder,
  UVT_COUNT
};

class Model;
class Shader;

class UVWrap
{
public:
  UVWrap(UVTypes type = UVT_Vertex, BoundingBox box = BoundingBox(), glm::vec2 offset = glm::vec2(0.0f, 0.0f), glm::vec2 scale = glm::vec2(1.0f, 1.0f));
  ~UVWrap() {};

  void CalculateUVontoModel(std::vector<Vertex> & verticesData);
  void UpdateUniform(Shader *shader);
  void SetBB(BoundingBox& box) { BB = box; };

  void SetUVScale(const glm::vec2& scale) { Scale = scale; };
  void SetUVShift(const glm::vec2& shift) { Offset = shift; };

  UVTypes GetType(void) { return type; };

  void DrawDebugMenu(Model *model);
private:
  UVTypes type;

  glm::vec2 Offset;
  glm::vec2 Scale;

  bool TextureEntityPosition;
  BoundingBox BB;
};

#endif