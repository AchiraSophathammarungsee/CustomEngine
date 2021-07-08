/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Framebuffer.h
Purpose: Wrapper for OpenGL framebuffer (init, bind, delete, etc.)
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/13/2019
End Header --------------------------------------------------------*/


#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include <glad/glad.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>

class Texture;
class TextureArray;

enum BufferAttachment
{
  BA_None = GL_NONE, BA_Color = GL_COLOR_ATTACHMENT0, BA_Depth = GL_DEPTH_ATTACHMENT
};

struct BufferTexture
{
public:
  BufferTexture(const std::string& name, GLuint id, int attachment, Texture* texture);
  ~BufferTexture();

  std::string Name;
  int Attachment;
  Texture* texture;
private:

};

class Shader;

class Framebuffer
{
public:
  Framebuffer(unsigned x, unsigned y, unsigned int width, unsigned int height, bool haveDepthBuffer = false, bool MSAA = false);
  ~Framebuffer();

  void ResizeBuffer(unsigned int width, unsigned int height);
  unsigned int GetWidht() { return Width; };
  unsigned int GetHeight() { return Height; };
  unsigned int GetPosX() { return PosX; };
  unsigned int GetPosY() { return PosY; };

  int AddBufferTexture(const std::string &name, Texture *texture, BufferAttachment attachment, GLuint attachmentOffset);
  int AddBufferArrayTexture(const std::string &name, TextureArray *texture, BufferAttachment attachment, GLuint attachmentOffset, unsigned int level);
  int AddEmptyTexture(const std::string &name, Texture *texture, BufferAttachment attachment, GLuint attachmentOffset);
  void RemoveBufferTexture(unsigned int id);
  void RemoveAllBufferTextures();

  void BindTexture(unsigned int attachmentID, unsigned int unit);
  void BindAllTextures(Shader *shader, int offset = 0);

  void AddDepthBuffer();
  bool HaveDepthBuffer() { return haveDepthBuffer; };
  void CopyDepthBuffer(unsigned offsetX, unsigned offsetY);
  void CopyDepthBuffer(Framebuffer *target, unsigned offsetX, unsigned offsetY);

  void DrawToBuffer(BufferAttachment attachment, int offset);
  GLuint GetFBO() { return FBO; };

  void BeginCapture(bool clear = true);
  void EndCapture();

  void SetClearColor(const glm::vec4& color);
  void SetClearDepth(float value);

  const BufferTexture& GetTexture(unsigned index) const { return Textures[index]; };
  void SetTexture(unsigned index, Texture *texture);
  void SetArrayTexture(unsigned index, TextureArray*texture, unsigned int level);

  void DrawDebugMenu();
private:
  GLuint FBO;
  std::vector<BufferTexture> Textures;
  unsigned int PosX, PosY, Width, Height;
  bool haveDepthBuffer;
  bool MSAA;
  glm::vec4 ClearColor;
  float ClearDepthValue;
};

#endif