/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Cubemap.h
Purpose: Encapsulate skybox behaviours (cube-textures, skybox model, etc.)
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/15/2019
End Header --------------------------------------------------------*/


#ifndef CUBEMAP_H
#define CUBEMAP_H
#include "Texture.h"

class Model;

class CubeMap : public Texture
{
public:
  CubeMap(unsigned int width, unsigned int height, TextureEdgeBehavior edgebehavior = TEB_Wrap, TextureApproxBehavior approxbehavior = TAB_Nearest, TextureComponent format = TC_RGBA, TextureComponent internalFormat = TC_RGBA, GLDataType dataType = DT_uByte);
  CubeMap(const std::string& filename, TextureEdgeBehavior edgebehavior = TEB_Wrap, TextureApproxBehavior approxbehavior = TAB_Nearest, TextureComponent format = TC_RGBA, TextureComponent internalFormat = TC_RGBA, GLDataType dataType = DT_uByte);
  
  ~CubeMap();

  void SetEdgeBehavior(TextureEdgeBehavior behavior);
  void SetApproxBehavior(TextureApproxBehavior behavior);
  bool LoadTexture(const std::string& filename);

  void Bind(GLuint unit = 0) override;

  void DrawDebugMenu(void);
private:
  void RegisterTexture(GLubyte* bytes);
};

#endif