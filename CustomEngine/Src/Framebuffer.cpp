/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Framebuffer.cpp
Purpose: Wrapper for OpenGL framebuffer (init, bind, delete, etc.)
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/13/2019
End Header --------------------------------------------------------*/
#include "Framebuffer.h"
#include "shader.hpp"
#include "EngineCore.h"
#include "Window.h"
#include "Texture.h"

BufferTexture::BufferTexture(const std::string& name, GLuint id, int attachment, Texture* texture) :
  Name(name), Attachment(attachment), texture(texture)
{

}

BufferTexture::~BufferTexture()
{
}

Framebuffer::Framebuffer(unsigned x, unsigned y, unsigned int width, unsigned int height, bool haveDepthBuffer, bool msaa) : PosX(x), PosY(y), Width(width), Height(height), 
  haveDepthBuffer(haveDepthBuffer), MSAA(msaa), ClearColor(), ClearDepthValue(1.f)
{
  glGenFramebuffers(1, &FBO);
  
  if (haveDepthBuffer)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);

    if(MSAA)
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, Width, Height);
    else
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Width, Height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    LOG_ERROR("Frame buffer", "Framebuffer is not complete!");
}

Framebuffer::~Framebuffer()
{
  glDeleteFramebuffers(1, &FBO);
  
  for (BufferTexture& texture : Textures)
  {
    delete texture.texture;
  }
}

void Framebuffer::ResizeBuffer(unsigned int width, unsigned int height)
{
  Width = width; Height = height;

  for (BufferTexture& texture : Textures)
  {
    texture.texture->Resize(Width, Height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, texture.Attachment, GL_TEXTURE_2D, texture.texture->GetID(), 0);
  }
}

int Framebuffer::AddBufferTexture(const std::string& name, Texture* texture, BufferAttachment attachment, GLuint attachmentOffset)
{
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);

  /*if (texture->GetWidth() != Width || texture->GetHeight() != Height)
  {
    texture->Resize(Width, Height);
  }*/
  BufferTexture bufferTexture(name, 0, attachment + attachmentOffset, texture);

  /*glGenTextures(1, &texture.ID);
  glBindTexture(GL_TEXTURE_2D, texture.ID);
  glTexImage2D(GL_TEXTURE_2D, 0, texture.internalFormat, Width, Height, 0, texture.format, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);*/
  //bufferTexture.texture->Bind();

  if(MSAA)
    glFramebufferTexture2D(GL_FRAMEBUFFER, bufferTexture.Attachment, GL_TEXTURE_2D_MULTISAMPLE, bufferTexture.texture->GetID(), 0);
  else
  {
    if (bufferTexture.texture->GetType() == TT_Cube || bufferTexture.texture->GetType() == TT_2D_ARRAY)
    {
      glFramebufferTexture(GL_FRAMEBUFFER, bufferTexture.Attachment, bufferTexture.texture->GetID(), 0);
    }
    else
    {
      glFramebufferTexture2D(GL_FRAMEBUFFER, bufferTexture.Attachment, bufferTexture.texture->GetType(), bufferTexture.texture->GetID(), 0);
    }
  }

  int id = Textures.size();
  Textures.push_back(bufferTexture);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return id;
}

int Framebuffer::AddBufferArrayTexture(const std::string& name, TextureArray* texture, BufferAttachment attachment, GLuint attachmentOffset, unsigned int level)
{
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);

  BufferTexture bufferTexture(name, 0, attachment + attachmentOffset, (Texture*)texture);

  glFramebufferTextureLayer(GL_FRAMEBUFFER, bufferTexture.Attachment, bufferTexture.texture->GetID(), 0, level);

  int id = Textures.size();
  Textures.push_back(bufferTexture);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return id;
}

int Framebuffer::AddEmptyTexture(const std::string& name, Texture* texture, BufferAttachment attachment, GLuint attachmentOffset)
{
  BufferTexture bufferTexture(name, 0, attachment + attachmentOffset, texture);

  int id = Textures.size();
  Textures.push_back(bufferTexture);

  return id;
}

void Framebuffer::SetTexture(unsigned index, Texture* texture)
{
  ASSERT(texture != nullptr, "Framebuffer: tried to set null texture");
  ASSERT(index < Textures.size(), "Framebuffer: tried to set texture with invalid index {}", index);

  Textures[index].texture = texture;

  glBindFramebuffer(GL_FRAMEBUFFER, FBO);
  if (MSAA)
    glFramebufferTexture2D(GL_FRAMEBUFFER, Textures[index].Attachment, GL_TEXTURE_2D_MULTISAMPLE, Textures[index].texture->GetID(), 0);
  else
  {
    if (Textures[index].texture->GetType() == TT_Cube)
    {
      glFramebufferTexture(GL_FRAMEBUFFER, Textures[index].Attachment, Textures[index].texture->GetID(), 0);
    }
    else
    {
      glFramebufferTexture2D(GL_FRAMEBUFFER, Textures[index].Attachment, Textures[index].texture->GetType(), Textures[index].texture->GetID(), 0);
    }
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::SetArrayTexture(unsigned index, TextureArray* texture, unsigned int level)
{
  ASSERT(texture != nullptr, "Framebuffer: tried to set null texture");
  ASSERT(index < Textures.size(), "Framebuffer: tried to set texture with invalid index {}", index);

  Textures[index].texture = (Texture*)texture;

  glBindFramebuffer(GL_FRAMEBUFFER, FBO);
  glFramebufferTextureLayer(GL_FRAMEBUFFER, Textures[index].Attachment, Textures[index].texture->GetID(), 0, level);
}

void Framebuffer::RemoveBufferTexture(unsigned int id)
{
  ASSERT(id < Textures.size(), "Framebuffer: tried to remove invalid texture attachment id {}", id);

  Textures.erase(Textures.begin() + id);
}

void Framebuffer::RemoveAllBufferTextures()
{
  Textures.clear();
}

void Framebuffer::BindTexture(unsigned int attachmentID, unsigned int unit)
{
  ASSERT(attachmentID < Textures.size(), "[Framebuffer] attempt to bind invalid attachment id {}", attachmentID);
  ASSERT(unit < 32, "[Framebuffer] attempt to bind attachment {}'s texture with invalid unit id {}", Textures[attachmentID].Name, unit);

  Textures[attachmentID].texture->Bind(unit);
}

void Framebuffer::BindAllTextures(Shader* shader, int offset)
{
  int unit = offset;
  for (BufferTexture& texture : Textures)
  {
    if (unit < 32)
    {
      texture.texture->Bind(unit);

      shader->use();
      std::string uniform("fbo_tex" + std::to_string(unit));
      shader->setInt(uniform, unit);
    }
    unit++;
  }
}

void Framebuffer::BeginCapture(bool clear)
{
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);
  glViewport(0, 0, Width, Height);
  if (clear)
  {
    glClearDepthf(ClearDepthValue);
    glClearColor(ClearColor.r, ClearColor.g, ClearColor.b, ClearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
}

void Framebuffer::EndCapture()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  /*glViewport(0, 0, Window::Width(), Window::Height());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
}

void Framebuffer::SetClearColor(const glm::vec4& color)
{
  ClearColor = color;
}

void Framebuffer::SetClearDepth(float value)
{
  ClearDepthValue = value;
}

void Framebuffer::DrawToBuffer(BufferAttachment attachment, int offset)
{
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);
  glDrawBuffer(attachment + offset);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::AddDepthBuffer()
{
  if (!haveDepthBuffer)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Width, Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    haveDepthBuffer = true;
  }
}

void Framebuffer::CopyDepthBuffer(unsigned offsetX, unsigned offsetY)
{
  // overwrite default framebuffer for forward rendering
  glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
  glBlitFramebuffer(0, 0, Width, Height, offsetX, offsetY, Width + offsetX, Height + offsetY, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Framebuffer::CopyDepthBuffer(Framebuffer *target, unsigned offsetX, unsigned offsetY)
{
  glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->FBO); // write to target framebuffer
  glBlitFramebuffer(0, 0, Width, Height, offsetX, offsetY, target->Width + offsetX, target->Height + offsetY, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void Framebuffer::DrawDebugMenu()
{
  for (BufferTexture& texture : Textures)
  {
    ImGui::Image(ImTextureID(texture.texture->GetID()), ImVec2(200.f, 200.f), ImVec2(0, 1), ImVec2(1, 0));
  }
}