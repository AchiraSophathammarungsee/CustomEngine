/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Textures.h
Purpose: Encapsulate all texture-related behaviours (load, init, draw, etc.)
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/14/2019
End Header --------------------------------------------------------*/

#ifndef TEXTURE_H
#define TEXTURE_H
#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>
#include <vector>

enum TextureEdgeBehavior
{
  TEB_Wrap,
  TEB_Clamp,
  TEB_COUNT
};

enum TextureApproxBehavior
{
  TAB_Linear,
  TAB_Nearest,
  TAB_LinearMipmap,
  TAB_COUNT
};

enum TextureFileFormat
{
  TFF_JPG,
  TFF_PNG,
  TFF_DDS,
  TFF_BMP,
  TFF_TGA,
  TFF_PSD,
  TFF_HDR
};

enum TextureComponent
{
  TC_RED = GL_RED, TC_R8 = GL_R8, TC_R16F = GL_R16F, TC_R32F = GL_R32F, TC_RG = GL_RG, TC_RG8SNORM = GL_RG8_SNORM, TC_RG16SNORM = GL_RG16_SNORM, TC_RGBA16SNORM = GL_RGBA16_SNORM, TC_RG32F = GL_RG32F, TC_RGB = GL_RGB, TC_RGB10 = GL_RGB10, TC_RGB16F = GL_RGB16F, TC_RGB32F = GL_RGB32F, TC_RGBA = GL_RGBA, TC_RGBA8 = GL_RGBA8, TC_RGBA16F = GL_RGBA16F, TC_Depth = GL_DEPTH_COMPONENT, TC_Depth16 = GL_DEPTH_COMPONENT16, TC_Depth32F = GL_DEPTH_COMPONENT32F
};

enum GLDataType
{
  DT_Int = GL_INT, DT_uInt = GL_UNSIGNED_INT, DT_Float = GL_FLOAT, DT_Double = GL_DOUBLE, DT_Byte = GL_BYTE, DT_uByte = GL_UNSIGNED_BYTE
};

enum TextureTypes
{
  TT_1D = GL_TEXTURE_1D, TT_2D = GL_TEXTURE_2D, TT_2D_ARRAY = GL_TEXTURE_2D_ARRAY, TT_3D = GL_TEXTURE_3D, TT_Cube = GL_TEXTURE_CUBE_MAP, TT_2DMSAA = GL_TEXTURE_2D_MULTISAMPLE
};

class Shader;
class Transform;
class Framebuffer;

class Texture
{
public:
  Texture(TextureTypes type = TT_2D, TextureEdgeBehavior edgebehavior = TEB_Wrap, TextureApproxBehavior approxbehavior = TAB_Nearest, TextureComponent format = TextureComponent::TC_RGBA, TextureComponent internalFormat = TextureComponent::TC_RGBA, GLDataType dataType = GLDataType::DT_uByte);
  Texture(GLuint width, GLuint height, TextureTypes type = TT_2D, TextureEdgeBehavior edgebehavior = TEB_Wrap, TextureApproxBehavior approxbehavior = TAB_Nearest, TextureComponent format = TextureComponent::TC_RGBA, TextureComponent internalFormat = TextureComponent::TC_RGBA, GLDataType dataType = GLDataType::DT_uByte);
  Texture(const std::string& filename, TextureTypes type = TT_2D, TextureEdgeBehavior edgebehavior = TEB_Wrap, TextureApproxBehavior approxbehavior = TAB_Nearest, TextureComponent format = TextureComponent::TC_RGBA, TextureComponent internalFormat = TextureComponent::TC_RGBA, GLDataType dataType = GLDataType::DT_uByte);
  Texture(GLubyte* bytes, GLuint width, GLuint height, TextureTypes type = TT_2D, GLuint channels = 4, TextureEdgeBehavior edgebehavior = TEB_Wrap, TextureApproxBehavior approxbehavior = TAB_Nearest, TextureComponent format = TextureComponent::TC_RGBA, TextureComponent internalFormat = TextureComponent::TC_RGBA, GLDataType dataType = GLDataType::DT_uByte);
  Texture(GLubyte* bytes, GLuint width, GLuint height, TextureTypes type = TT_2D, TextureComponent format = TextureComponent::TC_RGBA, TextureComponent internalFormat = TextureComponent::TC_RGBA, GLDataType dataType = GLDataType::DT_uByte, GLuint channels = 4, TextureEdgeBehavior edgebehavior = TEB_Wrap, TextureApproxBehavior approxbehavior = TAB_Nearest);
  Texture(std::vector<glm::vec3> &data, GLuint width, GLuint height, TextureTypes type = TT_2D, TextureComponent format = TextureComponent::TC_RGBA, TextureComponent internalFormat = TextureComponent::TC_RGBA, GLDataType dataType = GLDataType::DT_uByte, TextureEdgeBehavior edgebehavior = TEB_Wrap, TextureApproxBehavior approxbehavior = TAB_Nearest);
  virtual ~Texture();
  void DeleteTexture();

  bool LoadTexture(const std::string& filename);
  virtual void Bind(GLuint unit = 0);
  void GenerateMipmap();

  const std::string& GetPath(void) const { return Path; };

  TextureTypes GetType() const { return type; };

  GLuint GetWidth(void) const { return Width; };
  GLuint GetHeight(void) const { return Height; };
  GLuint GetID(void) const { return ID; };
  TextureEdgeBehavior GetEdgeBehavior(void) const { return EdgeBehavior; };
  TextureApproxBehavior GetApproxBehavior(void) const { return ApproxBehavior; };
  void SetEdgeBehavior(TextureEdgeBehavior behavior);
  void SetApproxBehavior(TextureApproxBehavior behavior);
  void SetBorderColor(float* borderColor);

  TextureComponent GetFormat() const { return Format; };
  TextureComponent GetInternalFormat() const { return InternalFormat; };
  GLDataType GetDataType() const { return DataType; };

  void UpdateGPU();
  void UpdateGPU(unsigned xOffset, unsigned yOffset, unsigned width, unsigned height);
  void UpdateGPU(GLubyte *bytes, unsigned xOffset, unsigned yOffset, unsigned width, unsigned height);
  void Resize(unsigned width, unsigned height);

  GLubyte* GetBytes() { return Bytes; };

  bool textureExist() { return textureOnGPU; };
  void DrawDebugMenu();

  static void GeneratePreview(Texture* texture, Framebuffer* preview, Shader* previewShader, Transform *transform) {};
  static void DrawResourceCreator(bool &openFlag);

protected:
  TextureTypes type;
  std::string Path;
  GLuint ID;
  int Width, Height, Channels;
  TextureEdgeBehavior EdgeBehavior;
  TextureApproxBehavior ApproxBehavior;
  GLubyte* Bytes;
  bool textureOnGPU;
  TextureComponent Format, InternalFormat;
  GLDataType DataType;
  int ArraySize;

private:
  void RegisterTexture(GLubyte* bytes);
  void RegisterTexture(std::vector<glm::vec3> &data);
  void RegisterTextureArray(GLubyte* bytes);
};

#endif
