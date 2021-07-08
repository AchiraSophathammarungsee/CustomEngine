/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: DepthMap.vert
Purpose: For render light's shadow depth map
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/15/2019
End Header --------------------------------------------------------*/

#version 410 core

layout (std140) uniform Matrices
{
    mat4 projMatrix;
};

uniform mat4 modelMatrix;
uniform mat4 lightSpaceMatrix;

uniform mat4 bones[64];

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3  vPosition;
layout(location = 5) in vec4 vBoneIDs;
layout(location = 6) in vec4 vWeights;

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
  
  gl_Position = lightSpaceMatrix * modelMatrix * boneTransform * vec4(vPosition, 1.0f );
}
