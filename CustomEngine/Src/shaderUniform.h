#pragma once
#ifndef SHADERUNIFORM_H
#define SHADERUNIFORM_H

#include <glm/glm.hpp>
#include <string>

struct boolUniform
{
  boolUniform() : id(-1), name(), value(false) {};

  int id;
  const char* name;
  bool value;

  void DrawDebugMenu();
};

struct intUniform
{
  intUniform() : id(-1), name(), value() {};

  int id;
  const char* name;
  int value;

  void DrawDebugMenu();
};

struct floatUniform
{
  floatUniform() : id(-1), name(), value() {};

  int id;
  const char* name;
  float value;

  void DrawDebugMenu();
};

struct vec2Uniform
{
  vec2Uniform() : id(-1), name(), value() {};

  int id;
  const char* name;
  glm::vec2 value;

  void DrawDebugMenu();
};

struct vec3Uniform
{
  vec3Uniform() : id(-1), name(), value(), isColor(false) {};

  int id;
  const char* name;
  glm::vec3 value;
  bool isColor;

  void DrawDebugMenu();
};

struct vec4Uniform
{
  vec4Uniform() : id(-1), name(), value(), isColor(false) {};

  int id;
  const char* name;
  glm::vec4 value;
  bool isColor;

  void DrawDebugMenu();
};

struct mat3Uniform
{
  mat3Uniform() : id(-1), name(), value(1.f) {};

  int id;
  const char* name;
  glm::mat3 value;

  void DrawDebugMenu();
};

struct mat4Uniform
{
  mat4Uniform() : id(-1), name(), value(1.f) {};

  int id;
  const char* name;
  glm::mat4 value;

  void DrawDebugMenu();
};

class Texture;
struct tex1DUniform
{
  tex1DUniform() : id(-1), name(), value(nullptr), bindID(-1) {};

  int id;
  const char* name;
  int bindID;
  Texture *value;

  void DrawDebugMenu();
};

struct tex2DUniform
{
  tex2DUniform() : id(-1), name(), value(nullptr), bindID(-1) {};

  int id;
  const char* name;
  int bindID;
  Texture* value;

  void DrawDebugMenu();
};

struct tex3DUniform
{
  tex3DUniform() : id(-1), name(), value(nullptr), bindID(-1) {};

  int id;
  const char* name;
  int bindID;
  Texture* value;

  void DrawDebugMenu();
};

class CubeMap;
struct texCubeUniform
{
  texCubeUniform() : id(-1), name(), value(nullptr), bindID(-1) {};

  int id;
  const char* name;
  int bindID;
  CubeMap* value;

  void DrawDebugMenu();
};

#endif