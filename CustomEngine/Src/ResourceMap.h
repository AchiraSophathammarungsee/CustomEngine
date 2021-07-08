#ifndef RESOURCEMAP_H
#define RESOURCEMAP_H

#include <map>
#include <string>
#include "EngineCore.h"
#include "Transform.h"
#include "Framebuffer.h"
#include "TextureAtlas.h"
#include "Rect.h"
#include "CString.h"
#include "Editor.h"
#include "shader.hpp"

class Shader;

template<typename T>
struct Resource
{
  Resource(T* data, const char *name) : Data(data), Name(name), PreviewID(0)
  {
    
  }

  ~Resource()
  {
    Clear();
  }

  void Clear()
  {
    delete Data;
    Data = nullptr;
  }

  std::string Name;
  unsigned int PreviewID;
  T* Data;
};

class IResourceMap
{
public:
  IResourceMap(unsigned int id, const char* typeName) : ID(id), TypeName(typeName), PreviewTransform(Transform(glm::vec3(0.f, 0.f, -1.f))), PreviewCount(0), PreviewFB(nullptr), PreviewShader(nullptr), PreviewTexture(nullptr), HavePreview(false) {};

  virtual void Clear() = 0;

  void SetHavePreview(bool flag) { HavePreview = flag; };
  bool IsHavePreview() { return HavePreview; };

  void CreatePreview();

  void SetPreviewShader(Shader* shader) { PreviewShader = shader; }
  TextureAtlas* GetPreviewTexture() { return PreviewTexture; };
  const char* GetTypeName() const { return TypeName; };

  virtual void DrawDebugMenu(Rect<unsigned int> window) {};
  virtual void DrawEditorMenu(Rect<unsigned int> window) {};
protected:
  void SetResourceManagerDisplay(unsigned int typeId);

  unsigned int ID;
  const char* TypeName;
  bool HavePreview;
  Shader* PreviewShader;
  Transform PreviewTransform;
  Framebuffer* PreviewFB;
  TextureAtlas* PreviewTexture;
  unsigned int PreviewCount;
};

template<typename T>
class ResourceMap : public IResourceMap
{
public:
  ResourceMap(unsigned int id, const char *typeName) : IResourceMap(id, typeName), EditSelect(nullptr), EditName("")
  {
  };

  ~ResourceMap() {};

  bool Add(T* resource, const char *name)
  {
    if (auto it = resourceMap.find(name) == resourceMap.end())
    {
      Resource<T> *r = new Resource<T>(resource, name);
      if(HavePreview) GeneratePreview(r);
      resourceMap[name] = r;
      return true;
    }
    LOG_ERROR_S("ResourceMap", "Trying to add duplicated resource: {}", name);
    return false;
  }

  T* Get(const char *name)
  {
    auto it = resourceMap.find(name);
    if (it != resourceMap.end())
    {
      return it->second->Data;
    }
    else
    {
      LOG_ERROR_S("ResourceMap", "Trying to get invalid resource: {}", name);
      return nullptr;
    }
  }

  bool Remove(const char *name)
  {
    auto it = resourceMap.find(name);
    if (it != resourceMap.end())
    {
      delete it->second;
      resourceMap.erase(it);
      return true;
    }
    else
    {
      LOG_ERROR_S("ResourceMap", "Trying to remove invalid resource: {}", name);
      return false;
    }
  }

  void Clear() override
  {
    for (auto it = resourceMap.begin(); it != resourceMap.end(); it++)
    {
      delete it->second;
    }

    resourceMap.clear();
  }

  unsigned int Size() const
  {
    return resourceMap.size();
  }

  std::map<std::string, Resource<T>*>& GetMap() { return resourceMap; };

  void DrawDebugMenu(Rect<unsigned int> window) override
  {
    static bool addingResource = false;
    if (ImGui::Button(CString::Add("Add ", TypeName)))
    {
      addingResource = true;
    }
    ImGui::Separator();

    unsigned int i = 0;
    for (auto it = resourceMap.begin(); it != resourceMap.end(); it++)
    {
      if (i * 120 < window.ScaleX())
        ImGui::SameLine();
      else
        i = 0;

      ImGui::BeginChild(CString::Add(it->first.c_str(), "frame"), ImVec2(100.f, 120.f), true);

      /*if (HavePreview)
      {
        glm::vec4 tileuv = PreviewTexture->GetTileUV(it->second->PreviewID);
        ImVec2 topright(tileuv.x + tileuv.z, tileuv.y);
        ImVec2 bottomleft(tileuv.x, tileuv.y - tileuv.w);

        if (ImGui::ImageButton(ImTextureID(PreviewTexture->GetTexture()->GetID()), ImVec2(80.f, 80.f), bottomleft, topright))
          EditResource(it->second->Data, it->first.c_str());
      }
      else
      {
        if (ImGui::Button(CString::Add(it->first.c_str(), i), ImVec2(80.f, 80.f)))
          EditResource(it->second->Data, it->first.c_str());
      }*/
      if (ImGui::Button(CString::Add(it->first.c_str(), i), ImVec2(80.f, 80.f)))
        EditResource(it->second->Data, it->first.c_str());
      ImGui::Text(it->first.c_str());

      ImGui::EndChild();
      i++;
    }
    if (addingResource)
    {
      T::DrawResourceCreator(addingResource);
    }
  }

  void EditResource(T* resource, const char *name)
  {
    Editor::SelectMenu("Right Editor", "Resource Editor");
    SetResourceManagerDisplay(ID);

    EditSelect = resource;
    EditName = name;
  }

  void DrawEditorMenu(Rect<unsigned int> window) override
  {
    if (EditSelect == nullptr) return;

    ImGui::Text("Name: %s", EditName);
    EditSelect->DrawDebugMenu();
  }

private:
  void GeneratePreview(Resource<T>* resource)
  {
    T::GeneratePreview(resource->Data, PreviewFB, PreviewShader, &PreviewTransform);

    //PreviewTexture->WriteToTexture(PreviewFB->GetTexture(0).texture, PreviewCount);
    resource->PreviewID = PreviewCount;
    PreviewCount++;
  }

  std::map<std::string, Resource<T>*> resourceMap;

  T* EditSelect;
  const char* EditName;
};

#endif