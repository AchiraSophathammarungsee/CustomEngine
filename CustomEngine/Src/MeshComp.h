
#ifndef MESHCOMP_H
#define MESHCOMP_H

#include "BoundingVolume.h"
#include "BoundingBox.h"

class Model;
class Material;

class MeshComp
{
public:
  MeshComp(Model* model = nullptr, Material* material = nullptr);
  ~MeshComp();

  Model* mesh;
  Material* material;
  BoundingVolume boundingVolume;
  BoundingBox BB;

  bool hide;
  
  void DrawDebugMenu();
};

#endif