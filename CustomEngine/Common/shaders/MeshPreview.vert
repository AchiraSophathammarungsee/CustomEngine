/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Phong.vert
Purpose: Render basic Phong lighting model + multiple lights support
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/14/2019
End Header --------------------------------------------------------*/

#version 410 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3  vPosition;
layout(location = 1) in vec3  vNormal;
layout(location = 2) in vec3  vTangent;
layout(location = 3) in vec3  vBitangent;
layout(location = 4) in vec2  vUV;

uniform mat4 modelMatrix;

out vec3 Normal;
out vec3 FragPos; 

void main()
{
    vec4 worldPos = modelMatrix * vec4(vPosition.x, -vPosition.y, vPosition.z, 1.0f );
    gl_Position = worldPos;
    
    FragPos = vec3(worldPos.xyz);
    Normal = vNormal;
}
