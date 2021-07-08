#include "TextureLoader.h"
#include "Texture.h"
#include "Cubemap.h"
#include "Framebuffer.h"
#include "EngineCore.h"
#include "shader.hpp"
#include "Model.h"
#include "ResourceManager.h"

#include <stb_image.h>

static Framebuffer* cubemapCapture;
static Framebuffer* convoluteCubemap;
static Shader* hdrToCubeShader;
static Shader* convoluteCubeShader;
static Shader* prefilterCubeShader;
static Shader* genBRDFShader;
static Model* cube;
static Model* quad;

unsigned int captureFBO, captureRBO;

glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
glm::mat4 captureViews[] =
{
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};

namespace TextureLoader
{
  void Init(Shader *hdrToCube, Shader *convolute, Shader* prefilter, Shader* genBRDF, Model *c)
  {
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    hdrToCubeShader = hdrToCube;
    convoluteCubeShader = convolute;
    prefilterCubeShader = prefilter;
    genBRDFShader = genBRDF;
    cube = c;

    quad = ResourceManager::Get<Model>("quad");
  }

  void Exit()
  {
    glDeleteBuffers(1, &captureRBO);
    glDeleteBuffers(1, &captureFBO);
  }

  CubeMap* LoadHDR(const char* filepath)
  {
    //stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float* data = stbi_loadf(filepath, &width, &height, &nrComponents, 0);
    unsigned int hdrTexture = 0;

    ASSERT(data, "HDR Loader: failed to load hdr file {}", stbi_failure_reason());

    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    CubeMap* result = new CubeMap(1024, 1024, TEB_Clamp, TAB_LinearMipmap, TC_RGB, TC_RGB, DT_Float);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
    // convert HDR equirectangular environment map to cubemap equivalent
    glViewport(0, 0, 1024, 1024); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    hdrToCubeShader->use();
    hdrToCubeShader->setMat4("projection", captureProjection);
    hdrToCubeShader->setInt("hdrTex", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    for (unsigned int i = 0; i < 6; ++i)
    {
      hdrToCubeShader->setMat4("view", captureViews[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, result->GetID(), 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      cube->Draw();
    }
    
    glDeleteTextures(1, &hdrTexture);

    result->GenerateMipmap();

    return result;
  }

  float* LoadHDRRaw(const char* filepath, int &width, int &height)
  {
    int nrComponents;
    float* data = stbi_loadf(filepath, &width, &height, &nrComponents, 0);
    unsigned int hdrTexture = 0;

    ASSERT(data, "HDR Loader: failed to load hdr file {}", stbi_failure_reason());

    return data;
  }

  void FreeSTBIImage(float* image)
  {
    stbi_image_free(image);
  }

  CubeMap* ConvoluteCubemap(CubeMap* target)
  {
    CubeMap* result = new CubeMap(32, 32, TEB_Clamp, TAB_Linear, TC_RGB, TC_RGB, DT_Float);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    convoluteCubeShader->use();
    convoluteCubeShader->setMat4("projection", captureProjection);
    convoluteCubeShader->setInt("envMap", 0);
    target->Bind(0);
    for (unsigned int i = 0; i < 6; ++i)
    {
      convoluteCubeShader->setMat4("view", captureViews[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, result->GetID(), 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      cube->Draw();
    }
    return result;
  }

  CubeMap* PrefilterCubemap(CubeMap* target)
  {
    CubeMap* result = new CubeMap(128, 128, TEB_Clamp, TAB_LinearMipmap, TC_RGB, TC_RGB, DT_Float);

    prefilterCubeShader->use();
    prefilterCubeShader->setInt("envMap", 0);
    prefilterCubeShader->setMat4("projection", captureProjection);
    target->Bind(0);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
      // reisze framebuffer according to mip-level size.
      unsigned int mipWidth = 128 * std::pow(0.5, mip);
      unsigned int mipHeight = 128 * std::pow(0.5, mip);
      glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
      glViewport(0, 0, mipWidth, mipHeight);

      float roughness = (float)mip / (float)(maxMipLevels - 1);
      prefilterCubeShader->setFloat("roughness", roughness);
      for (unsigned int i = 0; i < 6; ++i)
      {
        prefilterCubeShader->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, result->GetID(), mip);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube->Draw();
      }
    }

    return result;
  }

  Texture* GenerateBRDF()
  {
    Texture* brdf = new Texture(nullptr, 512, 512, TT_2D, 2, TEB_Wrap, TAB_Linear, TC_RG, TC_RG, DT_Float);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf->GetID(), 0);

    glViewport(0, 0, 512, 512);
    genBRDFShader->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    quad->Draw();

    return brdf;
  }
}