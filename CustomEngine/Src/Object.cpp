#include "Object.h"
#include "EngineCore.h"

Object::Object(const std::string& name) : Name(name)
{
  strcpy_s(nameBuffer, Name.c_str());
}

void Object::DrawDebugMenu()
{
  if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
  {
    Name = nameBuffer;
  }
}