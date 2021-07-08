#include "SceneManager.h"
#include "Log.h"
#include "Time.h"
#include "Profiler.h"

Scene* SceneManager::CurrentScene;

void SceneManager::Init(Scene* newscene)
{
  LOG_TRACE("SceneManager", "Initializing scene...");

  MeasureClock clock("Init scene");
  Time::Init();

  CurrentScene = nullptr;

  LoadScene(newscene);
}

void SceneManager::Update()
{
  // update frame dt
  Time::Update();
  Time::AccumDT() += Time::DeltaTime();

  CurrentScene->ProcessInput();

  Profiler::Begin("Fixed Update");

  // update with fixed dt (physics simulation)
  Time::ResetFixedFrameCount();
  while (Time::AccumDT() >= Time::FixedDeltaTime())
  {
    CurrentScene->FixedUpdate();

    Time::AccumDT() -= Time::FixedDeltaTime();
    Time::IncrementFixedFrameCount();
  }
  Time::UpdateTimeStepRatio();

  Profiler::End();

  // update with frame dt
  CurrentScene->Update();

  // render
  CurrentScene->Display();
}

void SceneManager::Exit()
{
  LOG_TRACE("SceneManager", "Exit, cleaning up scene...");

  if (CurrentScene)
  {
    CurrentScene->CleanUp();
    delete CurrentScene;
  }

  CurrentScene = nullptr;
}

// Reload current scene
void SceneManager::RefreshScene()
{
  CurrentScene->CleanUp();
  CurrentScene->Init();
}

// Load new scene
void SceneManager::LoadScene(Scene* nextscene)
{
  if (!nextscene)
  {
    LOG_ERROR("SceneManager", "Tried to load nullpointer scene.");
    return;
  }

  if (CurrentScene)
  {
    CurrentScene->CleanUp();
  }

  CurrentScene = nextscene;
  CurrentScene->Init();
}

Scene* SceneManager::GetCurrentScene()
{
  return CurrentScene;
}

   