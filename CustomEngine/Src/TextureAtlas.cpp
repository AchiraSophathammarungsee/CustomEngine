#include "TextureAtlas.h"
#include "ResourceManager.h"
#include "shader.hpp"
#include "Model.h"
#include "Window.h"
#include "Framebuffer.h"

TextureAtlas::Tile::Tile(TextureAtlas* _parent, unsigned size, const glm::vec2& topLeftPos) : parent(_parent)
{
  float uvScale = (float)size / (float)parent->GetResolution();
  float halfScale = uvScale / 2.f;
  float uvPosX = topLeftPos.x + halfScale;
  float uvPosY = topLeftPos.y - halfScale;

  transform.SetScale(glm::vec3(uvScale, uvScale, 1.f));
  transform.SetPosition(glm::vec3(uvPosX, uvPosY, 0.f));
  transform.UpdateMatrix();

  UV.x = uvPosX;
  UV.y = uvPosY;
  UV.z = uvScale;
  UV.w = uvScale;
}

TextureAtlas::TextureAtlas(Shader* _writeShader, unsigned int size, TextureComponent internalFormat, TextureComponent format, GLDataType dataType)
  : Resolution(size), writeShader(_writeShader), gridNum(0), NextTilePos(glm::vec2(0.f)), NextTileMaxSize(size), Full(false)
{
  framebuffer = new Framebuffer(0, 0, Resolution, Resolution, false);
  framebuffer->AddBufferTexture("color", new Texture(nullptr, size, size, TT_2D, 4, TEB_Wrap, TAB_Nearest, format, internalFormat, dataType), BA_Color, 0);

  camMatrix = glm::lookAt(glm::vec3(0.5f, 0.5f, 0.f), glm::vec3(0.5f, 0.5f, -1.f), glm::vec3(0.f, 1.f, 0.f));

  quad = ResourceManager::GetQuad();
}

TextureAtlas::~TextureAtlas()
{
  delete framebuffer;
}

void TextureAtlas::WriteToTexture(GLuint textureID, unsigned tileID, unsigned subtileID)
{
  ASSERT(tileID < gridNum, "[Texture Atlas]: Attempt to write to invalidtile ID: {}", tileID);
  
  framebuffer->BeginCapture(false);

  writeShader->use();
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, textureID);

  writeShader->setMat4("modelMatrix", tiles[tileID].GetTransform().GetMatrix());
  writeShader->setInt("tex", 0);

  quad->Draw();

  framebuffer->EndCapture();
}

void TextureAtlas::WriteToTexture(Texture* texture, unsigned tileID, unsigned subtileID)
{
  WriteToTexture(texture->GetID(), tileID, subtileID);
}

void TextureAtlas::WriteToTexture(void* bytes, unsigned tileID, unsigned subtileID)
{
  if (tileID < gridNum)
  {
    //framebuffer->GetTexture(0).texture->UpdateGPU(reinterpret_cast<GLubyte*>(bytes), tileID % gridPerRow * tileSize, tileID / gridPerRow * tileSize, tileSize, tileSize);
  }
}

void TextureAtlas::BindTextures(unsigned offset)
{
  glActiveTexture(GL_TEXTURE0 + offset);
  glBindTexture(GL_TEXTURE_2D, framebuffer->GetTexture(0).texture->GetID());
}

const Texture* TextureAtlas::GetTexture() const
{ 
  return framebuffer->GetTexture(0).texture;
}

void TextureAtlas::AddTile(unsigned int size)
{
  ASSERT(size > Resolution, "[Texture Atlas] Attempt to add tile {} exceeding atlas size {}", size, Resolution);

  float uvScale = (float)size / (float)Resolution;


  if (NextTilePos.x + uvScale >= 1.f)
  {
    float nextY = NextTilePos.y - uvScale;
    ASSERT(nextY <= 0.f, "[Texture Atlas] Is full, can't add any more tiles");
    
    float nextX = 0.f;
    for (const Tile& t : tiles)
    {
      const glm::vec4& tileuv = t.GetUV();
      if (nextY > (tileuv.y + tileuv.w) && nextX < (tileuv.x + tileuv.z))
      {

      }
    }
  }
  NextTilePos += glm::vec2(size);
}

const TextureAtlas::Tile& TextureAtlas::GetTile(unsigned id) const
{
  ASSERT(id < gridNum, "[Texture Atlas] Attempted to get invalid tile id {}", id);
  return tiles[id];
}

void TextureAtlas::SetTile(unsigned id, const Tile& tile)
{

}

void TextureAtlas::Clear()
{

}

void TextureAtlas::DrawDebugMenu()
{
  static bool preview = false;
  static int previewTile = 0;
  //static float uvSize = 1.f / gridPerRow;
  if (ImGui::ImageButton(ImTextureID(framebuffer->GetTexture(0).texture->GetID()), ImVec2(300.f, 300.f)))
  {
    preview = true;
  }
  
  if (preview)
  {
    ImGui::SetNextWindowSize(ImVec2(1024, 1024));
    ImGui::Begin("Texture Atlas Preview", &preview);

    ImGui::Image(ImTextureID(framebuffer->GetTexture(0).texture->GetID()), ImVec2(1024, 1024));

    ImGui::End();
  }
  
}