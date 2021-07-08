/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Gbuffer.vert
Purpose: outputs each geomtry informations onto textures
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS350_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 1/21/2020
End Header --------------------------------------------------------*/

#version 430 core

layout (std140) uniform Matrices
{
  mat4 projMatrix;
};
uniform mat4 modelMatrix;

uniform mat4 bones[64];

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec2 vUV;
layout(location = 5) in vec4 vBoneIDs;
layout(location = 6) in vec4 vWeights;

out VS_OUT 
{
  vec3 posWS;
  vec3 ModelNorm;
  vec2 UV;
  mat3 TBN;
} vs_out;

void main()
{
  mat4 boneTransform = mat4(1.0);
  if(vWeights.x != 0.0)
  {
    boneTransform = bones[int(vBoneIDs.x)] * vWeights.x + 
                    bones[int(vBoneIDs.y)] * vWeights.y + 
                    bones[int(vBoneIDs.z)] * vWeights.z + 
                    bones[int(vBoneIDs.w)] * vWeights.w;
  }

  vec4 worldPos = modelMatrix * boneTransform * vec4(vPosition, 1.0f );
    
  vs_out.posWS = worldPos.xyz;
  vs_out.UV = vUV;
  vs_out.ModelNorm = (normalize(vNormal) + vec3(1.0f)) / 2.0f;
    
  vec3 T = vec3(modelMatrix * vec4(vTangent, 0.f));
  vec3 B = vec3(modelMatrix * vec4(vBitangent, 0.f));
  vec3 N = vec3(modelMatrix * vec4(vNormal, 0.f));
  vs_out.TBN = mat3(T, B, N);

  gl_Position = projMatrix * worldPos;
}
