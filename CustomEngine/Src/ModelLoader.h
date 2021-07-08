/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: ModelLoader.h
Purpose: Load model from obj file format
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/11/2019
End Header --------------------------------------------------------*/

#ifndef MODELLOADER_H
#define MODELLOADER_H

#include "Model.h"

class Model;
class SkinnedMesh;

struct PolyVertex
{
  PolyVertex(int vID = -1, int uID = -1, int nID = -1) : vertexID(vID), uvID(uID), normalID(nID) {};
  int vertexID, uvID, normalID;
};

struct Polygon
{
  std::vector<PolyVertex> vertices;
};

Model* LoadModelFile(const char* fileName, NormalBaseMode nbm, bool normalizeScale = true);
bool ReloadModel(const char* fileName, std::vector<Vertex> &outVertices, std::vector<Face> &outFaces, BoundingBox& outBox);

SkinnedMesh* ReadAssimpFile(const char* fileName);
Model* ReadObjFile(const char* fileName);

#endif //MODELLOADER_H
