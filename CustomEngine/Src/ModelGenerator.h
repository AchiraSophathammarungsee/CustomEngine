/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: ModelGenerator.h
Purpose: Procedurally generate sphere, ring, quad, and cube meshes
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/12/2019
End Header --------------------------------------------------------*/


#ifndef MODELGENERATOR_H
#define MODELGENERATOR_H
class Model;

namespace ModelGenerator
{
  Model* CreateSphere(float radius, int numDivisions);
  Model* CreateCircle(float radius, int segments);
  Model* CreateSquare();
  Model* CreateQuad();
  Model* CreateCube();
  Model* CreateLine();
  Model* CreateLineCube();
  Model* CreateLineSphere(int segments);
  Model* CreateLineTriangle();
  Model* CreateLineCircleSphere(float radius, int segments);
}

#endif
