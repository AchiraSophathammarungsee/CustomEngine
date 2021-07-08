/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Textures.cpp
Purpose: Encapsulate all texture-related behaviours (load, init, draw, etc.)
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/14/2019
End Header --------------------------------------------------------*/

#include "Texture.h"
#include "Model.h"
#include <SOIL.h>
#include "ResourceManager.h"
#include "FileDialog.h"

Texture::Texture(TextureTypes type, TextureEdgeBehavior edgebehavior, TextureApproxBehavior approxbehavior, TextureComponent format, TextureComponent internalFormat, GLDataType dataType) :
  EdgeBehavior(edgebehavior), ApproxBehavior(approxbehavior), textureOnGPU(false), Bytes(nullptr), ID(0), Width(0), Height(0), Channels(0), Format(format), InternalFormat(internalFormat), 
  DataType(dataType), type(type)
{

}

Texture::Texture(GLuint width, GLuint height, TextureTypes type, TextureEdgeBehavior edgebehavior, TextureApproxBehavior approxbehavior, TextureComponent format, TextureComponent internalFormat, GLDataType dataType) :
  EdgeBehavior(edgebehavior), ApproxBehavior(approxbehavior), textureOnGPU(false), Bytes(nullptr), ID(0), Width(width), Height(height), Channels(0), Format(format), InternalFormat(internalFormat),
  DataType(dataType), type(type)
{

}

Texture::Texture(const std::string& filename, TextureTypes type, TextureEdgeBehavior edgebehavior, TextureApproxBehavior approxbehavior, TextureComponent format, TextureComponent internalFormat, GLDataType dataType) :
  EdgeBehavior(edgebehavior), ApproxBehavior(approxbehavior), textureOnGPU(false), Bytes(nullptr), ID(0), Width(0), Height(0), Channels(0), Format(format), InternalFormat(internalFormat), 
  DataType(dataType), type(type)
{
  LoadTexture(filename);
}

Texture::Texture(GLubyte* bytes, GLuint width, GLuint height, TextureTypes type, GLuint channels, TextureEdgeBehavior edgebehavior, TextureApproxBehavior approxbehavior, TextureComponent format, TextureComponent internalFormat, GLDataType dataType) :
  EdgeBehavior(edgebehavior), ApproxBehavior(approxbehavior), Width(width), Height(height), Channels(channels), textureOnGPU(false), Bytes(nullptr), ID(0), Format(format), 
  InternalFormat(internalFormat), DataType(dataType), type(type)
{
  RegisterTexture(bytes);
}

Texture::Texture(GLubyte* bytes, GLuint width, GLuint height, TextureTypes type, TextureComponent format, TextureComponent internalFormat, GLDataType dataType, GLuint channels, TextureEdgeBehavior edgebehavior, TextureApproxBehavior approxbehavior) :
  EdgeBehavior(edgebehavior), ApproxBehavior(approxbehavior), Width(width), Height(height), Channels(channels), textureOnGPU(false), Bytes(nullptr), ID(0), Format(format), 
  InternalFormat(internalFormat), DataType(dataType), type(type)
{
  RegisterTexture(bytes);
}

Texture::Texture(std::vector<glm::vec3>& data, GLuint width, GLuint height, TextureTypes type, TextureComponent format, TextureComponent internalFormat, GLDataType dataType, TextureEdgeBehavior edgebehavior, TextureApproxBehavior approxbehavior) :
  EdgeBehavior(edgebehavior), ApproxBehavior(approxbehavior), Width(width), Height(height), textureOnGPU(false), Bytes(nullptr), ID(0), Format(format),
  InternalFormat(internalFormat), DataType(dataType), type(type)
{
  RegisterTexture(data);
}

Texture::~Texture()
{
  DeleteTexture();
}

void Texture::DeleteTexture()
{
  if (textureOnGPU)
  {
    glDeleteTextures(1, &ID);
    textureOnGPU = false;
  }
}

bool Texture::LoadTexture(const std::string &filename)
{
  Path = filename;

  GLubyte* bytes = SOIL_load_image(filename.c_str(), &Width, &Height, &Channels, SOIL_LOAD_AUTO);
  if (!bytes)
  {
    std::string error = SOIL_last_result();
    LOG_ERROR_S("Texture", "Failed to load texture file {}: {}", filename, error);
    
    GLubyte notexture[] = { 255, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 255, 255 };
    Width = 2; Height = 2;
    RegisterTexture(notexture);
    SOIL_free_image_data(bytes);
    return false;
  }

  switch (Channels)
  {
  case(1):
  {
    InternalFormat = TC_RED;
    Format = TC_RED;
    break;
  }
  case(2):
  {
    InternalFormat = TC_RG;
    Format = TC_RG;
    break;
  }
  case(3):
  {
    InternalFormat = TC_RGB;
    Format = TC_RGB;
    break;
  }
  case(4):
  {
    InternalFormat = TC_RGBA;
    Format = TC_RGBA;
    break;
  }
  }

  RegisterTexture(bytes);
  SOIL_free_image_data(bytes);
  return true;
}

void Texture::Bind(GLuint unit)
{
  if (unit < 32)
  {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(type, ID);
  }
}

void Texture::GenerateMipmap()
{
  glBindTexture(type, ID);
  glGenerateMipmap(type);
}

void Texture::SetEdgeBehavior(TextureEdgeBehavior behavior)
{
  EdgeBehavior = behavior;
  if (EdgeBehavior == TEB_Wrap)
  {
    glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  else
  {
    glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
};
void Texture::SetApproxBehavior(TextureApproxBehavior behavior)
{
  ApproxBehavior = behavior;
  if (ApproxBehavior == TAB_Linear)
  {
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else if (ApproxBehavior == TAB_LinearMipmap)
  {
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  }
  else
  {
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
};

void Texture::SetBorderColor(float *borderColor)
{
  glBindTexture(type, ID);
  glTexParameterfv(type, GL_TEXTURE_BORDER_COLOR, borderColor);
}

void Texture::RegisterTexture(GLubyte * bytes)
{
  if (textureOnGPU)
  {
    glDeleteTextures(1, &ID);
    textureOnGPU = false;
  }

  glGenTextures(1, &ID);
  glBindTexture(type, ID);

  SetEdgeBehavior(EdgeBehavior);
  SetApproxBehavior(ApproxBehavior);

  if(type == TT_2DMSAA)
    glTexImage2DMultisample(type, 4, InternalFormat, Width, Height, GL_TRUE);
  else
    glTexImage2D(type, 0, InternalFormat, Width, Height, 0, Format, DataType, bytes);

  textureOnGPU = true;
}

void Texture::RegisterTexture(std::vector<glm::vec3>& data)
{
  if (textureOnGPU)
  {
    glDeleteTextures(1, &ID);
    textureOnGPU = false;
  }

  glGenTextures(1, &ID);
  glBindTexture(type, ID);

  SetEdgeBehavior(EdgeBehavior);
  SetApproxBehavior(ApproxBehavior);

  if (type == TT_2DMSAA)
    glTexImage2DMultisample(type, 4, InternalFormat, Width, Height, GL_TRUE);
  else
    glTexImage2D(type, 0, InternalFormat, Width, Height, 0, Format, DataType, &data[0]);

  if (ApproxBehavior == TAB_LinearMipmap)
  {
    GenerateMipmap();
  }

  textureOnGPU = true;
}

void Texture::RegisterTextureArray(GLubyte* bytes)
{
  if (textureOnGPU)
  {
    glDeleteTextures(1, &ID);
    textureOnGPU = false;
  }

  glGenTextures(1, &ID);
  glBindTexture(type, ID);

  SetEdgeBehavior(EdgeBehavior);
  SetApproxBehavior(ApproxBehavior);

  int mipmaplevel = 8;
  glTexStorage3D(type, mipmaplevel, InternalFormat, Width, Height, ArraySize);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mipmaplevel, 0, 0, 0, Width, Height, ArraySize, Format, DataType, bytes);

  textureOnGPU = true;
}

void Texture::UpdateGPU()
{
  UpdateGPU(Bytes, 0, 0, Width, Height);
}

void Texture::UpdateGPU(unsigned xOffset, unsigned yOffset, unsigned width, unsigned height)
{
  UpdateGPU(Bytes, xOffset, yOffset, width, height);
}

void Texture::UpdateGPU(GLubyte *_bytes, unsigned xOffset, unsigned yOffset, unsigned width, unsigned height)
{
  if (textureOnGPU)
  {
    //memcpy_s(Bytes, sizeof(GLubyte) * Width * Height * Channels, _bytes, width * height * Channels);

    glBindTexture(type, ID);
    glTexSubImage2D(type, 0, xOffset, yOffset, width, height, Format, DataType, _bytes);
  }
}

void Texture::Resize(unsigned width, unsigned height)
{
  if (!textureOnGPU)
  {
    return;
  }

  glBindTexture(type, ID);
  glTexImage2D(type, 0, InternalFormat, width, height, 0, Format, DataType, Bytes);

  Width = width; Height = height;
}

void Texture::DrawDebugMenu()
{
  ImGui::Image((void*)(intptr_t)ID, ImVec2(128, 128));

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

  //static const char *typesStr[5] = {"1D", "2D", "3D", "Cube", "2D MSAA"};
  ImGui::Text("Type: %s", (const char*)glGetString(type));

  static const char *edgebehaviours[TEB_COUNT] = {"Wrap", "Clamp"};
  static const char *approxbehaviours[TAB_COUNT] = {"Linear", "Nearest"};
  static const char *formats[5] = {"RGB", "RGB16F", "RGB32F", "RGBA", "Depth"};
  static const char *dataTypes[6] = {"Int", "Unsigned Int", "Float", "Double", "Byte", "Unsigned Byte"};
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

void Texture::DrawResourceCreator(bool &openFlag)
{
  static char texNameBuffer[256] = "unnamed";
  static char texPathBuffer[270] = "../Common/textures/";

  ImGui::SetNextWindowSize(ImVec2(300, 200));
  ImGui::Begin("New Texture", &openFlag);

  if (ImGui::Button("Open..."))
  {
    std::string path = FileDialog::OpenFile("Texture Files\0*.jpg;*.png;*.dds;*.tif;*.bmp;*.tga\0");
    strcpy_s(texPathBuffer, 270, path.c_str());
  }

  ImGui::InputText("texture name", texNameBuffer, 255);
  ImGui::InputText("texture path", texPathBuffer, 270);

  if (ImGui::Button("Create Texture"))
  {
    ResourceManager::Add<Texture>(new Texture(texPathBuffer), texNameBuffer);
    openFlag = false;
  }

  ImGui::End();
}