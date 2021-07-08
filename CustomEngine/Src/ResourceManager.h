#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

//#include "Material.h"
#include <unordered_map>
#include <string>
#include "EngineCore.h"
#include "ResourceMap.h"
#include "Rect.h"

class Shader;
class Model;
class Texture;
class Material;
class SkinnedMesh;
class AudioClip;

class ResourceManager
{
public:
  static void Init();
  static void Exit();

  template<typename T>
  static void RegisterResource()
  {
    const char* typeName = typeid(T).name();

    ASSERT(TypeToID.find(typeName) == TypeToID.end(), "[ResourceManager] Attempt to register resource map of type: {} twice.", typeName);

    TypeToID.insert({ typeName, NextResourceMapID });
    ResourceMaps.push_back(new ResourceMap<T>(NextResourceMapID, typeName));

    NextResourceMapID++;

    LOG_TRACE_S("ResourceManager", "Registered resource map: {}", typeName);
  }

  template<typename T>
  static T *Get(const char *name)
  {
    return GetResourceMap<T>()->Get(name);
  }

  template<typename T>
  static bool Add(T *resource, const char* name)
  {
    return GetResourceMap<T>()->Add(resource, name);
  };

  template<typename T>
  static bool Remove(const char* name)
  {
    return GetResourceMap<T>()->Remove(name);
  };

  /*static ResourceMap<Texture> textures;
  static ResourceMap<Model> meshes;
  static ResourceMap<Material> materials;
  static ResourceMap<Shader> shaders;*/

  static Model* GetQuad() { return Quad; };

  static void DrawDebugMenu(Rect<unsigned int> window);
  /*static void EditTexture(Texture *texture, const std::string &name);
  static void EditModel(Model*mesh, const std::string& name);
  static void EditMaterial(Material*material, const std::string& name);
  static void EditShader(Shader*shader, const std::string& name);*/
  static void DrawEditorMenu(Rect<unsigned int> window);
  /*static void DrawTextureSelector(Texture **outTexture, bool *windowFlag);
  static void DrawMeshSelector(Model ** outModel, bool *windowFlag);
  static void DrawMaterialSelector(Material** outMaterial, bool *windowFlag);*/

  template<typename T>
  static unsigned int GetTypeID()
  {
    const char* typeName = typeid(T).name();
    auto typePair = TypeToID.find(typeName);

    ASSERT(typePair != TypeToID.end(), "[ResourceManager] Attempt to get type id of: {} before register.", typeName);

    return typePair->second;
  }

  template<typename T>
  static void DrawResourceSelector(T** output, bool* windowFlag)
  {
    ResourceMap<T>* map = GetResourceMap<T>();

    ImGui::SetNextWindowSize(ImVec2(300, 300));
    ImGui::Begin("Resource Selector");

    TextureAtlas* icons = map->GetPreviewTexture();
    int i = 0;
    for (auto it = map->GetMap().begin(); it != map->GetMap().end(); it++)
    {
      if (i % 3) ImGui::SameLine();

      //if (map->IsHavePreview())
      //{
      //  glm::vec4 tileuv = icons->GetTileUV(it->second->PreviewID);
      //  ImVec2 topright(tileuv.x + tileuv.z, tileuv.y);
      //  ImVec2 bottomleft(tileuv.x, tileuv.y - tileuv.w);

      //  /*if (ImGui::ImageButton(ImTextureID(icons->GetTexture()->GetID()), ImVec2(80.f, 80.f), bottomleft, topright))
      //  {
      //    *output = it->second->Data;
      //    *windowFlag = false;
      //  }*/
      //  if (ImGui::Button(it->first.c_str()))
      //  {
      //    *output = it->second->Data;
      //    *windowFlag = false;
      //  }
      //}
      //else
      //{
      //  if (ImGui::Button(it->first.c_str()))
      //  {
      //    *output = it->second->Data;
      //    *windowFlag = false;
      //  }
      //}
      if (ImGui::Button(it->first.c_str()))
      {
        *output = it->second->Data;
        *windowFlag = false;
      }

      i++;
    }
    ImGui::End();
  }

  template<typename T>
  static ResourceMap<T>* GetResourceMap()
  {
    const char* typeName = typeid(T).name();
    auto typePair = TypeToID.find(typeName);

    ASSERT(typePair != TypeToID.end(), "[ResourceManager] Attempt to get resource map: {} before register.", typeName);

    return reinterpret_cast<ResourceMap<T>*>(ResourceMaps[typePair->second]);
  };

  static void SetEditType(unsigned int typeId);

  template<typename T>
  static void SetEdit(T *editResource, const char *editName) 
  { 
    ResourceMap<T>* map = GetResourceMap<T>();

    map->EditResource(editResource, editName);
  };

private:
  static Texture* editTexture;
  static Model* editModel;
  static Material* editMaterial;
  static Shader* editShader;
  static SkinnedMesh* editSkinnedMesh;
  static std::string editName;

  static std::vector<IResourceMap*> ResourceMaps;
  static std::unordered_map<const char*, unsigned int> TypeToID;
  static unsigned int NextResourceMapID;
  static unsigned int EditorTypeID;

  static Model* Quad;

  ResourceManager() {};
  ~ResourceManager() {};
};

#endif