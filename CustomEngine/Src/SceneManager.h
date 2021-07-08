
#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "Scene.h"

class SceneManager
{
public:
  static void Init(Scene* newscene);
  static void Update();
  static void Exit();

  // Reload current scene
  static void RefreshScene();
  // Load new scene
  static void LoadScene(Scene* nextscene);

  static Scene* GetCurrentScene();

private:
  SceneManager() {};
  ~SceneManager() {};

  static Scene* CurrentScene;
};

#endif