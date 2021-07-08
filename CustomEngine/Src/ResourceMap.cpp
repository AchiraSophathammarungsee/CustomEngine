#include "ResourceMap.h"
#include "Editor.h"
#include "ResourceManager.h"

void IResourceMap::CreatePreview()
{
  HavePreview = true;

  PreviewFB = new Framebuffer(0, 0, 200, 200);
  PreviewFB->AddBufferTexture("color channel", new Texture(nullptr, 200, 200, TT_2D, TC_RGBA, TC_RGBA, DT_uByte), BA_Color, 0);

  //PreviewTexture = new TextureAtlas(ResourceManager::Get<Shader>("texture atlas"), 4096, 32, TC_RGBA, TC_RGBA, DT_uByte);
}

void IResourceMap::SetResourceManagerDisplay(unsigned int typeId)
{
  ResourceManager::SetEditType(typeId);
}

//template<typename T>
//void ResourceMap<T>::DrawDebugMenu(Rect<unsigned int> window)
//{
//  static bool addingResource = false;
//  if (ImGui::Button(CString::Add("Add ", TypeName)))
//  {
//    addingResource = true;
//  }
//  ImGui::Separator();
//
//  unsigned int i = 0;
//  for (auto it = resourceMap.begin(); it != resourceMap.end(); it++)
//  {
//    if (i * 100 < window.ScaleX())
//      ImGui::SameLine();
//    else
//      i = 0;
//
//    ImGui::BeginChild(CString::Add(it->first.c_str(), "frame"), ImVec2(100.f, 120.f), true);
//
//    if (HavePreview)
//    {
//      glm::vec4 tileuv = PreviewTexture->GetTileUV(it->second->PreviewID);
//      ImVec2 topright(tileuv.x + tileuv.z, tileuv.y);
//      ImVec2 bottomleft(tileuv.x, tileuv.y - tileuv.w);
//
//      if (ImGui::ImageButton(ImTextureID(PreviewTexture->GetTexture()->GetID()), ImVec2(80.f, 80.f), bottomleft, topright))
//        EditResource(it->second->Data, it->first.c_str());
//    }
//    else
//    {
//      if(ImGui::Button(CString::Add(it->first.c_str(), i), ImVec2(80.f, 80.f)))
//        EditResource(it->second->Data, it->first.c_str());
//    }
//    ImGui::Text(it->first.c_str());
//    
//    ImGui::EndChild();
//    i++;
//  }
//  if (addingResource)
//  {
//    T::DrawResourceCreator(addingResource);
//  }
//}
//
//template<typename T>
//void ResourceMap<T>::EditResource(T* resource, const char *name)
//{
//  Editor::SelectMenu("Right Editor", "Resource Editor");
//  SetResourceManagerDisplay(ID);
//
//  EditSelect = resource;
//  EditName = name;
//}
//
//template<typename T>
//void ResourceMap<T>::DrawEditorMenu(Rect<unsigned int> window)
//{
//  ImGui::Text("Name: %s", EditName.c_str());
//  EditSelect->DrawDebugMenu();
//}