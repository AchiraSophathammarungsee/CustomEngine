#ifndef TEXTUREATLAS_H
#define TEXTUREATLAS_H

#include "Texture.h"
#include <vector>
#include <glm/glm.hpp>
#include "Transform.h"

class Model;
class Shader;
class Framebuffer;

class TextureAtlas
{
  struct Tile
  {
  public:
    Tile(TextureAtlas* parent, unsigned size, const glm::vec2 &topLeftPos);

    Transform& GetTransform() { return transform; };
    const glm::vec4& GetUV() const { return UV; };

    void Clear();

  private:
    Transform transform;
    TextureAtlas* parent;
    unsigned id;
    glm::vec4 UV;
  };

public:
  TextureAtlas(Shader *writeShader, unsigned int size, TextureComponent internalFormat, TextureComponent format, GLDataType dataType);
  ~TextureAtlas();

  void WriteToTexture(Texture *texture, unsigned tileID, unsigned subtileID = 0);
  void WriteToTexture(GLuint textureID, unsigned tileID, unsigned subtileID = 0);
  void WriteToTexture(void *bytes, unsigned tileID, unsigned subtileID = 0);
  void BindTextures(unsigned offset);
  const Texture* GetTexture() const;
  Framebuffer* GetFrambuffer() { return framebuffer; };

  unsigned int GetResolution() const { return Resolution; };

  void AddTile(unsigned int size);
  unsigned int GetTileSize() const { return tileSize; };
  unsigned int GetGridNum() const { return gridNum; };
  const glm::vec4& GetTileUV(unsigned int tileID) const { return GetTile(tileID).GetUV(); };

  const Tile &GetTile(unsigned id) const;
  void SetTile(unsigned id, const Tile &tile);
  
  void Clear();

  void DrawDebugMenu();

private:
  Framebuffer *framebuffer;
  glm::mat4 camMatrix;
  unsigned int gridNum;
  unsigned int tileSize;
  unsigned int Resolution;
  glm::vec2 NextTilePos;
  unsigned int NextTileMaxSize;
  bool Full;
  std::vector<Tile> tiles;
  Model* quad;
  Shader* writeShader;
};

#endif