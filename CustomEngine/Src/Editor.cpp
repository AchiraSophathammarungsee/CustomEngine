#include "Editor.h"
#include "EngineCore.h"
#include "Window.h"
#include "FileDialog.h"
#include "Profiler.h"

std::vector<EditorMenu> Editor::Menus;
DrawFunction Editor::ObjEditor;
WindowFunction Editor::Windows[WS_COUNT];
bool Editor::WindowsOpen[WS_COUNT];

void Editor::Init()
{
  LOG_TRACE("Editor", "Initializing Editor...");
  ObjEditor = nullptr;

  Windows[WS_Time] = Time::DrawDebugMenu;
  WindowsOpen[WS_Time] = false;

  Windows[WS_Profiler] = Profiler::DrawDebugMenu;
  WindowsOpen[WS_Profiler] = false;
}

void Editor::Exit()
{
  LOG_TRACE("Editor", "Clearing Editor...");
  ObjEditor = nullptr;
  Menus.clear();
}

void Editor::DrawMenus()
{
  DrawFileMenu();
  DrawToolbar();

  for (unsigned int i = 0; i < Menus.size(); i++)
  {
    Menus[i].Draw();
  }

  for (size_t i = 0; i < WS_COUNT; i++)
  {
    if (Windows[i] && WindowsOpen[i])
    {
      Windows[i]();
    }
  }
}

void Editor::AddMenu(EditorMenu& menu)
{
  Menus.push_back(menu);
}

EditorMenu* Editor::GetMenu(const std::string& name)
{
  for (EditorMenu& menu : Menus)
  {
    if (menu.GetName() == name)
    {
      return &menu;
    }
  }

  LOG_TRACE_S("Editor", "Can't find menu with name: {}", name);
  return nullptr;
}

void Editor::SelectMenu(const std::string& menustr, const std::string& submenustr)
{
  EditorMenu* menu = GetMenu(menustr);
  if(menu)  menu->Select(submenustr);
}

void Editor::RemoveMenu(unsigned int id)
{
  if (id > Menus.size())
  {
    LOG_TRACE_S("Editor", "Can't erase menu with id: {}", id);
    return;
  }

  Menus.erase(Menus.begin() + id);
}

void EditorMenu::Draw()
{
  ImGui::SetNextWindowPos(ImVec2(Window.TopLeftX(), Window.TopLeftY()));
  ImGui::SetNextWindowSize(ImVec2(Window.ScaleX(), Window.ScaleY()));
  ImGui::Begin(Name.c_str(), &Open, ImGuiWindowFlags_NoTitleBar);

  ImGui::BeginTabBar(Name.c_str());
  if (Menus.empty() == false)
  {
    for (menuMap::iterator it = Menus.begin(); it != Menus.end(); it++)
    {
      if (ImGui::BeginTabItem(it->first.c_str()))
      {
        MenuSelect = it->second.funcPtr;

        ImGui::EndTabItem();
      }

      ImGui::SameLine();
    }
    ImGui::Separator();

    if (MenuSelect)
    {
      MenuSelect(Window);
    }
  }
  ImGui::EndTabBar();

  ImGui::End();
}

void EditorMenu::Add(const std::string& name, DrawFunction drawdebug)
{
  Menus[name] = MenuDrawer(drawdebug);
  MenuSelect = drawdebug;
}

void EditorMenu::Select(const std::string& name)
{
  menuMap::iterator it = Menus.find(name);
  if (it != Menus.end())
  {
    MenuSelect = it->second.funcPtr;
  }
}

void EditorMenu::Remove(const std::string& name)
{
  menuMap::iterator it = Menus.find(name);
  if (it != Menus.end())
  {
    Menus.erase(it);

    if (Menus.empty())
    {
      MenuSelect = nullptr;
    }
    else
    {
      MenuSelect = Menus.begin()->second.funcPtr;
    }
  }
}

void Editor::DrawFileMenu()
{
  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("New Scene"))
      {
        
      }
      if (ImGui::MenuItem("Open Scene"))
      {
        std::string filePath = FileDialog::OpenFile("Scene (*.scene)\0*.scene\0");
        if (filePath.empty() == false)
        {

        }
      }
      if (ImGui::MenuItem("Save Scene"))
      {
        std::string filePath = FileDialog::SaveFile("Scene (*.scene)\0*.scene\0");
        if (filePath.empty() == false)
        {

        }
      }
      if (ImGui::MenuItem("New Project"))
      {

      }
      if (ImGui::MenuItem("Open Project"))
      {

      }
      if (ImGui::MenuItem("Save Project"))
      {

      }
      if (ImGui::MenuItem("Build"))
      {

      }
      if (ImGui::MenuItem("Exit"))
      {

      }

      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit"))
    {
      if (ImGui::MenuItem("Undo"))
      {

      }
      if (ImGui::MenuItem("Redo"))
      {

      }
      if (ImGui::MenuItem("Preferences"))
      {

      }
      if (ImGui::MenuItem("Shortcuts"))
      {

      }

      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Assets"))
    {
      if (ImGui::MenuItem("Create Asset"))
      {

      }
      if (ImGui::MenuItem("Import Asset"))
      {

      }
      if (ImGui::MenuItem("Import Package"))
      {

      }
      if (ImGui::MenuItem("Export Package"))
      {

      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("GameObject"))
    {
      if (ImGui::MenuItem("Create Empty"))
      {

      }
      if (ImGui::MenuItem("Meshes"))
      {

      }
      if (ImGui::MenuItem("Lights"))
      {

      }
      if (ImGui::MenuItem("Effects"))
      {

      }
      if (ImGui::MenuItem("Audios"))
      {

      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Component"))
    {
      if (ImGui::MenuItem("Audios"))
      {

      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Window"))
    {
      if (ImGui::MenuItem("Layout"))
      {

      }
      if (ImGui::MenuItem("Time"))
      {
        WindowsOpen[WS_Time] = true;
      }
      if (ImGui::MenuItem("Profiler"))
      {
        WindowsOpen[WS_Profiler] = true;
      }
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}

void Editor::DrawToolbar()
{
  ImGui::SetNextWindowPos(ImVec2(0, 20));
  ImGui::SetNextWindowSize(ImVec2(Window::Width(), 40));

  static bool openToolbar = true;
  ImGui::Begin("Toolbar menu", &openToolbar, ImGuiWindowFlags_NoTitleBar);

  

  ImGui::End();
}

void Editor::DrawObjEditor(Rect<unsigned int> window)
{
  if (ObjEditor)
  {
    ObjEditor(window);
  }
}

void Editor::SetObjEditor(DrawFunction objEditor)
{
  ObjEditor = objEditor;
}