/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: PhongBlinn.vert
Purpose: Modify phong lighting to PhongBlinn model.
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/15/2019
End Header --------------------------------------------------------*/

#version 410 core

uniform mat4 modelMatrix;
uniform mat4 projMatrix;

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3  vPosition;
layout(location = 1) in vec3  vNormal;
layout(location = 2) in vec2  vUV;

out VS_OUT 
{
  vec3 Normal;
  vec3 FragPos;
} vs_out;

void main()
{
    vec4 worldPos = modelMatrix * vec4(vPosition, 1.0f );
    gl_Position = projMatrix * worldPos;
    
    vs_out.FragPos = vec3(worldPos);
    vs_out.Normal = vec3(modelMatrix * vec4(vNormal, 0.f));
}
