/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: GLApplication.cpp
Purpose: Modified to support multi-window drawing and other functionalities
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/11/2019
End Header --------------------------------------------------------*/

// Include standard headers
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstring>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Include GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "EngineCore.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Local / project headers
#include "shader.hpp"
#include "SimpleScene_Quad.h"
#include "Scene1.h"
#include "SceneManager.h"
#include "Window.h"
#include "Log.h"
#include "Time.h"
#include "AudioManager.h"
#include "Profiler.h"
#include "MathUtil.h"
#include "GraphicAPI.h"
//////////////////////////////////////////////////////////////////////

Scene  *scene;

int windowWidth = 1920;
int windowHeight = 1080;

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
int main()
{
    Log::Init();
    Log::SetOutputMode(Log::Console);

    WindowModes mode = WM_Windowed;
#ifdef MSAA_ON
    Window::Init(mode, 4);
#else
    Window::Init(mode, 1);
#endif

    LOG_TRACE("App", "Initializing GLEW...");
    //LOG_WARN("App", "Consider changing to GLAD...");

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return -1;
    }

    const char* cardVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* graphicsCard = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    LOG_TRACE("Driver", cardVendor);
    LOG_TRACE("Driver", graphicsCard);
    int glMajorVersion = 0, glMinorVersion = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
    LOG_TRACE_S("Driver", "GL Version: {}.{}", glMajorVersion, glMinorVersion);

    LOG_TRACE("OpenGL", "Enabled cube map seamless");
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    if (cardVendor == "Nvidia")
    {
      int glTotMem = 0, glCurrMem = 0;
      //glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &glTotMem);
      //glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &glCurrMem);
      //LOG_TRACE_S("Driver", "Video Memory: {}/{}", glCurrMem, glTotMem);
    }
    else if(cardVendor == "ATI" || cardVendor == "AMD")
    {
      /*GLuint uNoOfGPUs = wglGetGPUIDsAMD(0, 0);
      GLuint* uGPUIDs = new GLuint[uNoOfGPUs];
      wglGetGPUIDsAMD(uNoOfGPUs, uGPUIDs);

      GLuint uTotalMemoryInMB = 0;
      wglGetGPUInfoAMD(uGPUIDs[0], WGL_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof(GLuint), &uTotalMemoryInMB);

      GLint nCurAvailMemoryInKB = 0;
      glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, &nCurAvailMemoryInKB);*/
    }

    AudioManager::Init();

    LOG_TRACE("ImGUI", "Hook IMGUI to GLFW");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(Window::GetWindowHandler(), true);
    ImGui_ImplOpenGL3_Init("#version 410");
    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.085f, 0.11f, 1.f);

    style.Colors[ImGuiCol_Header] = ImVec4(0.21f, 0.205f, 0.23f, 1.f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.31f, 0.305f, 0.33f, 1.f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.2505f, 0.253f, 1.f);

    style.Colors[ImGuiCol_Button] = ImVec4(0.21f, 0.205f, 0.23f, 1.f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.31f, 0.305f, 0.33f, 1.f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.2505f, 0.253f, 1.f);

    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.21f, 0.205f, 0.23f, 1.f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.31f, 0.305f, 0.33f, 1.f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.2505f, 0.253f, 1.f);

    style.Colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.1505f, 0.153f, 1.f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.39f, 0.3805f, 0.383f, 1.f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.29f, 0.2805f, 0.283f, 1.f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.16f, 0.1505f, 0.153f, 1.f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.21f, 0.205f, 0.23f, 1.f);

    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.1505f, 0.153f, 1.f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.26f, 0.2505f, 0.253f, 1.f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.96f, 0.1505f, 0.953f, 1.f);

    io.Fonts->AddFontFromFileTTF("../Common/fonts/Open_Sans/OpenSans-Bold.ttf", 18.f);
    io.FontDefault = io.Fonts->AddFontFromFileTTF("../Common/fonts/Open_Sans/OpenSans-Regular.ttf", 18.f);

    MathUtil::InitRand();
    SceneManager::Init(new Scene1());

    do
    {
      Window::MakeCurrentContext();
      Window::PoleEvents();

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      Profiler::FrameBegin();

      SceneManager::Update();

      AudioManager::Update();

      Profiler::FrameEnd();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      Window::SwapBuffer();

    } // Check if the ESC key was pressed or the window was closed
    while (Window::GetKey(GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           Window::ShouldClose() == 0);

    SceneManager::Exit();

    // ImGui Cleanup
    LOG_TRACE("ImGUI", "Shuting down IMGUI...");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    AudioManager::Exit();

    // Close OpenGL window and terminate GLFW
    Window::Exit();

    Log::Clear();

    return 0;
}
