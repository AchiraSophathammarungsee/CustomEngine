#pragma once
#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include <string>

class Material;
class Framebuffer;
class Model;

class PostProcess
{
public:
  // nullptr framebuffer = src
  PostProcess(const char *name, Material* material, Framebuffer* input = nullptr, Framebuffer* output = nullptr);

  void Draw(Model *quad);

  Material* GetMaterial() const { return material; };
  Framebuffer* GetInput() const { return input; };
  Framebuffer* GetOutput() const { return output; };

  void SetInput(Framebuffer* in) { input = in; };
  void SetOutput(Framebuffer* out) { output = out; };

  void DrawDebugMenu();

private:
  std::string name;
  Material* material;
  Framebuffer* input;
  Framebuffer* output;
};

#endif