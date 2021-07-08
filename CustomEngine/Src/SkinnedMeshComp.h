#pragma once
#ifndef SKINNEDMESHCOMP_H
#define SKINNEDMESHCOMP_H

class SkinnedMesh;
class Material;

class SkinnedMeshComp
{
public:
  SkinnedMeshComp(SkinnedMesh* model = nullptr, Material* material = nullptr);
  ~SkinnedMeshComp();

  SkinnedMesh* mesh;
  Material* material;

  bool hide;

  void DrawDebugMenu();
};

#endif