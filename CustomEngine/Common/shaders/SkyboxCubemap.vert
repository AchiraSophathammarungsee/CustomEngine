/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Skybox.vert
Purpose: For render skybox
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/15/2019
End Header --------------------------------------------------------*/

#version 410 core
layout(location = 0) in vec3 vPosition;

out vec3 TexCoords;

layout (std140) uniform Matrices
{
  mat4 finalMatrix;
  mat4 projMatrix;
  mat4 viewMatrix;
};

uniform mat4 skyMatrix;

void main()
{
    TexCoords = 0.5 * vPosition;
    gl_Position = projMatrix * mat4(mat3(viewMatrix)) * vec4(vPosition, 1.0);
    gl_Position = gl_Position.xyww;
}  