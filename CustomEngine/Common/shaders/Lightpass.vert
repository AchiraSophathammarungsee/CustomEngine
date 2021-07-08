/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Lightpass.vert
Purpose: Calculate lighting from g-buffers
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS350_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 1/21/2020
End Header --------------------------------------------------------*/

#version 410 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3  vPosition;
layout(location = 1) in vec3  vNormal;
layout(location = 2) in vec3  vTangent;
layout(location = 3) in vec3  vBitangent;
layout(location = 4) in vec2  vUV;

out vec2 TexCoords;

void main()
{
    TexCoords = vUV;
    gl_Position = vec4(2.f * vPosition, 1.0f);
}
