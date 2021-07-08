#pragma once
#ifndef SKYBOX_H
#define SKYBOX_H

class Model;
class Material;
class Light;
class CubeMap;

class Skybox
{
public:
  Skybox(Material *mat = nullptr);
  ~Skybox() {};

  Material* GetMaterial() { return m_material; };
  void SetMaterial(Material* mat) { m_material = mat; };
  CubeMap* GetCubeMap();

  void Draw();

  void DrawDebugMenu();

private:
  Model* m_cube;
  Material* m_material;
  Light* m_sun;
  Light* m_moon;
};

#endif