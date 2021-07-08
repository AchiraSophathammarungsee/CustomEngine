//
// Created by pushpak on 3/28/18.
//

#ifndef SCENE_H
#define SCENE_H

#include "Entity.h"

class Scene
{

public:
    Scene();
    virtual ~Scene();

    void AddEntity();

    virtual void ProcessInput() {};

    // Init: called once when the scene is initialized
    virtual int Init();

    virtual void Update() {};

    virtual void FixedUpdate() {};

    // LoadAllShaders: This is the placeholder for loading the shader files
    virtual void LoadAllShaders();

    // Display : encapsulates per-frame behavior of the scene
    virtual int Display();

    // preRender : called to setup stuff prior to rendering the frame
    virtual int preRender();

    // Render : per frame rendering of the scene
    virtual int Render();

    // postRender : Any updates to calculate after current frame
    virtual int postRender();

    // cleanup before destruction
    virtual void CleanUp();

protected:

};


#endif //SAMPLE3_2_FBO_SCENE_H
