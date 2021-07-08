#include "FileDialog.h"

#include <Windows.h>
#include <direct.h>
#define GetCurrentDir _getcwd
#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Window.h"

namespace FileDialog
{
  const char InitDir[MAX_PATH] = "";

  size_t GetMaxFileBuffer()
  {
    return MAX_PATH;
  }

  std::string CurrPath()
  {
    TCHAR buffer[MAX_PATH] = { 0 };
    GetCurrentDir(buffer, MAX_PATH);

    return std::string(buffer);
  }

  std::string OpenFile(const char* filter)
  {
    OPENFILENAMEA ofn; // dialog box struct
    CHAR szFile[MAX_PATH] = { 0 }; // if use TCHAR macros

    // initialize OPENFILENAMEA
    ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = glfwGetWin32Window(Window::GetWindowHandler());
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrInitialDir = CurrPath().c_str();
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileName(&ofn) == TRUE)
    {
      return ofn.lpstrFile;
    }
    return std::string();
  }

  std::string SaveFile(const char* filter)
  {
    OPENFILENAMEA ofn; // dialog box struct
    CHAR szFile[MAX_PATH] = { 0 }; // if use TCHAR macros

    // initialize OPENFILENAMEA
    ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = glfwGetWin32Window(Window::GetWindowHandler());
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrInitialDir = CurrPath().c_str();
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetSaveFileName(&ofn) == TRUE)
    {
      return ofn.lpstrFile;
    }
    return std::string();
  }
}