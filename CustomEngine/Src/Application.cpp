#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Log.h"
#include "Time.h"
#include "Application.h"
#include "Scene1.h"

Application* Application::StaticInstance = nullptr;

Application::Application(const std::string& name)
{
  if (StaticInstance)
  {
    // app already exist!!
  }

  StaticInstance = this;

  Window::Init();

  LOG_TRACE("App", "Initializing GLEW...");
  LOG_WARN("App", "Consider changing to GLAD...");
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return;
  }

  const char* cardVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
  const char* graphicsCard = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
  LOG_TRACE("Driver", cardVendor);
  LOG_TRACE("Driver", graphicsCard);
  int glMajorVersion = 0, glMinorVersion = 0;
  glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
  glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
  LOG_TRACE_S("Driver", "GL Version: {}.{}", glMajorVersion, glMinorVersion);
  

  if (cardVendor == "Nvidia")
  {
    int glTotMem = 0, glCurrMem = 0;
    //glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &glTotMem);
    //glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &glCurrMem);
    //LOG_TRACE_S("Driver", "Video Memory: {}/{}", glCurrMem, glTotMem);
  }
  else if (cardVendor == "ATI" || cardVendor == "AMD")
  {
    /*GLuint uNoOfGPUs = wglGetGPUIDsAMD(0, 0);
    GLuint* uGPUIDs = new GLuint[uNoOfGPUs];
    wglGetGPUIDsAMD(uNoOfGPUs, uGPUIDs);

    GLuint uTotalMemoryInMB = 0;
    wglGetGPUInfoAMD(uGPUIDs[0], WGL_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof(GLuint), &uTotalMemoryInMB);

    GLint nCurAvailMemoryInKB = 0;
    glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, &nCurAvailMemoryInKB);*/
  }

  LOG_TRACE("ImGUI", "Hook IMGUI to GLFW");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;

  ImGui_ImplGlfw_InitForOpenGL(Window::GetWindowHandler(), true);
  ImGui_ImplOpenGL3_Init("#version 410");
  ImGui::StyleColorsDark();

  // Initialize the scene
  scene = new Scene1();

  // Scene::Init encapsulates setting up the geometry and the texture
  // information for the scene
  scene->Init();
}

void Application::Run()
{
  while (Running)
  {
    Time::Update();

    Window::MakeCurrentContext();
    Window::PoleEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Now render the scene
    // Scene::Display method encapsulates pre-, render, and post- rendering operations
    scene->ProcessInput();
    scene->Display();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    Window::SwapBuffer();
  }
}

void Application::Close()
{
  scene->CleanUp();

  // ImGui Cleanup
  LOG_TRACE("ImGUI", "Shuting down IMGUI...");
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  // Close OpenGL window and terminate GLFW
  Window::Exit();
}