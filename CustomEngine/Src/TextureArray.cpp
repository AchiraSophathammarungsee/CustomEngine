#include "TextureArray.h"
#include "EngineCore.h"

TextureArray::TextureArray(unsigned int layer, unsigned int width, unsigned int height, TextureTypes type, TextureComponent format, TextureComponent internalFormat, GLDataType dataType, TextureEdgeBehavior edgebehavior, TextureApproxBehavior approxbehavior)
  : Texture(width, height, type, edgebehavior, approxbehavior, format, internalFormat, dataType), Layers(layer)
{
  Register();
}

void TextureArray::Register()
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

  glTexImage3D(type,
    0,                 // mipmap level
    InternalFormat,          // gpu texel format
    Width,             // width
    Height,             // height
    Layers,             // depth
    0,                 // border
    Format,      // cpu pixel format
    DataType,  // cpu pixel coord type
    nullptr);           // pixel data

  glTexParameteri(type, GL_TEXTURE_MAX_LEVEL, Layers);

  textureOnGPU = true;
}

void TextureArray::SetTexture(unsigned int layer, GLubyte* data)
{
  ASSERT(textureOnGPU, "[Texture Array] tried to set texture to unregistered texture array");
  ASSERT(layer < Layers, "[Texture Array] tried to set texture with invalid layer id");

  glTexSubImage3D(type, 0, 0, 0, 0, Width, Height, layer, Format, DataType, data);
}