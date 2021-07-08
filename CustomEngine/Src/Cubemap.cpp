/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Cubemap.cpp
Purpose: Encapsulate skybox behaviours (cube-textures, skybox model, etc.)
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/15/2019
End Header --------------------------------------------------------*/

#include "Cubemap.h"
#include <SOIL.h>
#include <imgui.h>

CubeMap::CubeMap(unsigned int width, unsigned int height, TextureEdgeBehavior edgebehavior, TextureApproxBehavior approxbehavior, TextureComponent format, TextureComponent internalFormat, GLDataType dataType) :
  Texture(TT_Cube, edgebehavior, approxbehavior, format, internalFormat, dataType)
{
  Width = width;
  Height = height;
  textureOnGPU = true;
  glGenTextures(1, &ID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
  for (unsigned int i = 0; i < 6; i++)
  {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, InternalFormat, Width, Height, 0, Format, DataType, nullptr);
  }

  SetEdgeBehavior(EdgeBehavior);
  SetApproxBehavior(ApproxBehavior);

  if (ApproxBehavior == TAB_LinearMipmap)
  {
    GenerateMipmap();
  }
}

CubeMap::CubeMap(const std::string& filename, TextureEdgeBehavior edgebehavior, TextureApproxBehavior approxbehavior, TextureComponent format, TextureComponent internalFormat, GLDataType dataType) :
  Texture(TT_Cube, edgebehavior, approxbehavior, format, internalFormat, dataType)
{
  LoadTexture(filename);
}

CubeMap::~CubeMap()
{
}

void CubeMap::Bind(GLuint unit)
{
  if (unit < 32)
  {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
  }
}

bool CubeMap::LoadTexture(const std::string & filename)
{
  if (textureOnGPU)
  {
    glDeleteTextures(1, &ID);
  }
  textureOnGPU = true;

  glGenTextures(1, &ID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

  Path = filename;

  std::vector<std::string> sides = { "_right", "_left", "_top", "_bottom", "_front", "_back" };
  
  for (unsigned int i = 0; i < sides.size(); i++)
  {
    std::string path = (filename + sides[i] + ".png");
    GLubyte * bytes = SOIL_load_image(path.c_str(), &Width, &Height, &Channels, SOIL_LOAD_RGBA);
    if (!bytes)
    {
      std::string error = SOIL_last_result();
      GLubyte notexture[] = { 255, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 255, 255 };
      Width = 2; Height = 2;
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
      SOIL_free_image_data(bytes);
      return false;
    }
    else
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
      SOIL_free_image_data(bytes);
    }
  }

  SetEdgeBehavior(EdgeBehavior);
  SetApproxBehavior(ApproxBehavior);
  return true;
}

void CubeMap::RegisterTexture(GLubyte * bytes)
{
  GLuint size = Height * Width * Channels;


}

void CubeMap::SetEdgeBehavior(TextureEdgeBehavior behavior)
{
  EdgeBehavior = behavior;
  if (EdgeBehavior == TEB_Wrap)
  {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
  }
  else
  {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  }
};
void CubeMap::SetApproxBehavior(TextureApproxBehavior behavior)
{
  ApproxBehavior = behavior;
  if (ApproxBehavior == TAB_Linear)
  {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else if (ApproxBehavior == TAB_LinearMipmap)
  {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  }
  else
  {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
};

void CubeMap::DrawDebugMenu()
{
  static char filePath[256];
  static bool init = true;
  if (init)
  {
    Path.copy(filePath, 256);
    init = false;
  }
  ImGui::InputText("File Path", filePath, 256);
  if (ImGui::Button("Load Texture"))
  {
    LoadTexture(filePath);
  }
  ImGui::Text("ID: %d", ID);
  ImGui::Text("Resolution: [%d, %d]", Width, Height);

  static const char* edgebehaviours[TEB_COUNT] = { "Wrap", "Clamp" };
  static const char* approxbehaviours[TAB_COUNT] = { "Linear", "Nearest" };
  static const char* formats[5] = { "RGB", "RGB16F", "RGB32F", "RGBA", "Depth" };
  static const char* dataTypes[6] = { "Int", "Unsigned Int", "Float", "Double", "Byte", "Unsigned Byte" };
  static int edgebehavior = EdgeBehavior;
  static int approxbehavior = ApproxBehavior;
  static int format = Format;
  static int internalformat = InternalFormat;
  static int datatype = DataType;

  if (ImGui::ListBox("Edge Behaviour", &edgebehavior, edgebehaviours, (int)TEB_COUNT))
    SetEdgeBehavior(static_cast<TextureEdgeBehavior>(edgebehavior));
  if (ImGui::ListBox("Approx Behaviour", &approxbehavior, approxbehaviours, (int)TAB_COUNT))
    SetApproxBehavior(static_cast<TextureApproxBehavior>(edgebehavior));
  if (ImGui::ListBox("Format", &format, formats, 5))
    Format = static_cast<TextureComponent>(format);
  if (ImGui::ListBox("Interal Format", &internalformat, formats, 5))
    InternalFormat = static_cast<TextureComponent>(internalformat);
  if (ImGui::ListBox("Data Type", &datatype, dataTypes, 6))
    DataType = static_cast<GLDataType>(datatype);
}