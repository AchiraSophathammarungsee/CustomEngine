#pragma once
#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include "SphericalHarmonics.h"

class Texture;
class CubeMap;
class Shader;
class Model;

namespace TextureLoader
{
  void Init(Shader* hdrToCube, Shader* convolute, Shader* prefilter, Shader *genBRDF, Model* cube);
  void Exit();

  Texture* Load2D(const char* filepath);
  CubeMap* LoadCubemap(const char* filepath);
  CubeMap* LoadHDR(const char *filepath);
  float* LoadHDRRaw(const char* filepath, int& width, int& height);

  Texture* GenerateBRDF();
  CubeMap* ConvoluteCubemap(CubeMap *target);
  CubeMap* PrefilterCubemap(CubeMap *target);

  void FreeSTBIImage(float *image);
}

#endif