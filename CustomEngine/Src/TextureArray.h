#pragma once
#ifndef TEXTUREARRAY_H
#define TEXTUREARRAY_H

#include "Texture.h"

class TextureArray : public Texture
{
public:
  TextureArray(unsigned int layer, unsigned int width, unsigned int height, TextureTypes type = TT_2D_ARRAY, TextureComponent format = TextureComponent::TC_RGBA, TextureComponent internalFormat = TextureComponent::TC_RGBA, GLDataType dataType = GLDataType::DT_uByte, TextureEdgeBehavior edgebehavior = TEB_Wrap, TextureApproxBehavior approxbehavior = TAB_Nearest);

  unsigned int GetLayerNum() const { return Layers; };
  float GetSampleDepth(unsigned int layer) const { return fmaxf(0.f, fminf(float(Layers) - 1.f, floorf((float)layer + 0.5f))); };

  void SetTexture(unsigned int layer, GLubyte *bytes);

private:
  void Register();

  unsigned int Layers;
};

#endif