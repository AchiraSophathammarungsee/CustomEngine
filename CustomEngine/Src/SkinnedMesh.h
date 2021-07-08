#pragma once
#ifndef SKINNEDMESH_H
#define SKINNEDMESH_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Vertex.h"
#include "MeshEnum.h"
#include "BoundingBox.h"
#include "BoneData.h"
#include "shader.hpp"
#include "UVWrap.h"
#include <map>
#include "SkeletalAnimation.h"

class Framebuffer;

class SkinnedMesh
{
public:
  SkinnedMesh(std::vector<SkinnedVertex>& _vertices, std::vector<unsigned int>& _indices, std::vector<Bone> &bones, const glm::mat4 &invRootMat, PrimitiveMode mode, bool dynamicDraw = false);
  ~SkinnedMesh();

  void UpdateBuffer();
  unsigned int GetVAO() const { return VAO; };
  unsigned int GetIndiceCount() const { return indicesBuffer.size(); };
  std::vector<SkinnedVertex>& GetVertices() { return verticesBuffer; };
  std::vector<unsigned>& GetIndices() { return indicesBuffer; };
  
  std::vector<Bone>& GetBones() { return bones; };
  Bone* getBone(const std::string& str) const { return boneMap.at(str); };
  const glm::mat4& getInverseRoot() const { return this->inverseRoot; };
  void SetAnimations(const std::vector<SkeletalAnimation>& anim) { animations = anim; };
  const std::vector<SkeletalAnimation>& GetAnimations() const { return animations; };
  bool HasAnimation() const { return animations.empty() == false; };
  void SetUseAnimation(bool flag) { useAnimations = flag; };
  bool IsUseAnimation() const { return useAnimations; };
  void SetUseIK(bool flag) { useIK = flag; };
  bool IsUseIK() const { return useIK; };
  void ForwardKinematic(Bone *bone);

  std::vector<glm::mat4>& GetFinalMatArray() { return finalMatrixArray; };

  void Draw();

  void SetFilePath(const std::string& path) { Path = path; };
  const std::string& GetFilePath(void) { return Path; };

  PrimitiveMode GetPrimitiveMode() const { return primitiveMode; };

  void SetBoundingBox(BoundingBox& box) { boundingBox = box; };
  const BoundingBox& GetBoundingBox(void) { return boundingBox; };

  bool ShowBones() const { return showBones; };
  void DrawDebugMenu();
  void PrintBones();

  static void GeneratePreview(SkinnedMesh* mesh, Framebuffer* preview, Shader* previewShader, Transform* transform);
  static void DrawResourceCreator(bool& openFlag);

private:
  void ClearBuffers();
  void RegisterBuffers();

  std::string Path;

  // actual vertices and indices buffer for rendering / sending to GPU
  std::vector<SkinnedVertex> verticesBuffer;
  std::vector<unsigned int> indicesBuffer;
  unsigned int VAO, VBO, EBO;
  PrimitiveMode primitiveMode;
  bool buffersOnGPU;
  bool dynamicDraw;

  glm::mat4 inverseRoot;
  std::vector<Bone> bones;
  std::map<std::string, Bone*> boneMap; // name to bone
  std::vector<SkeletalAnimation> animations;
  std::vector<glm::mat4> finalMatrixArray;
  bool useAnimations;
  bool useIK;
  bool showBones;

  BoundingBox boundingBox;
};

#endif