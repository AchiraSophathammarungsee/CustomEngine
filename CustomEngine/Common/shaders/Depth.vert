/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Depth.vert
Purpose: Render depth component of frag position
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_2
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 11/11/2019
End Header --------------------------------------------------------*/

#version 410 core

uniform mat4 modelMatrix;
uniform mat4 projMatrix;

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3  vPosition;
layout(location = 1) in vec3  vNormal;
layout(location = 2) in vec3  vTangent;
layout(location = 3) in vec3  vBitangent;
layout(location = 4) in vec2  vUV;

out VS_OUT 
{
  vec3 ModelPos;
  vec3 FragPos;
} vs_out;

void main()
{
    vec4 worldPos = modelMatrix * vec4(vPosition, 1.0f );
    gl_Position = projMatrix * worldPos;
    
    vs_out.ModelPos = vPosition;
    vs_out.FragPos = vec3(worldPos);
}
