/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Scene1.h
Purpose: Scene for current assigment
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/11/2019
End Header --------------------------------------------------------*/

#ifndef SCENE1_H
#define SCENE1_H

#include "Scene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include "Rect.h"

class Shader;
class DirectionalLight;
class SpotLight;
class PointLight;

class Scene1 : public Scene
{

public:
  Scene1();
  virtual ~Scene1();


public:
    int Init() override;
    void CleanUp() override;

    void Update() override;
    void FixedUpdate() override;

    int preRender() override;
    int Render() override;
    int postRender() override;

    void ProcessInput() override;
private:

    // member functions
    void initMembers();

    // This is the non-software engineered portion of the code
    // Go ahead and modularize the VAO and VBO setup into
    // BufferManagers and ShaderManagers
    void SetupModels();

    void DrawFileMenu();
    void DrawDebugMenu(Rect<unsigned int> window);

    std::vector<Shader*> shaders;
};


#endif //SCENE1_H
