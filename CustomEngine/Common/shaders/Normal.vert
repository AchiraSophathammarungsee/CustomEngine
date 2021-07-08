/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Normal.vert
Purpose: Render world-space normals
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_2
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 11/11/2019
End Header --------------------------------------------------------*/

#version 410 core

uniform mat4 modelMatrix;
uniform mat4 projMatrix;
uniform mat4 lightSpaceMatrix;

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3  vPosition;
layout(location = 1) in vec3  vNormal;
layout(location = 2) in vec3  vTangent;
layout(location = 3) in vec3  vBitangent;
layout(location = 4) in vec2  vUV;

out VS_OUT 
{
  vec3 ModelPos;
  vec3 ModelNorm;
  vec2 UV;
  vec3 FragPos; 
  vec4 FragPosLightSpace;
  mat3 TBN;
} vs_out;

void main()
{
    vec4 worldPos = modelMatrix * vec4(vPosition, 1.0f );
    gl_Position = projMatrix * worldPos;
    
    vs_out.ModelPos = vPosition;
    vs_out.FragPos = vec3(worldPos);
    vs_out.UV = vUV;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    vs_out.ModelNorm = (normalize(vNormal) + vec3(1.0f)) / 2.0f;
    
    vec3 T = vec3(modelMatrix * vec4(vTangent, 0.f));
    vec3 B = vec3(modelMatrix * vec4(vBitangent, 0.f));
    vec3 N = vec3(modelMatrix * vec4(vNormal, 0.f));
    vs_out.TBN = mat3(T, B, N);
}
