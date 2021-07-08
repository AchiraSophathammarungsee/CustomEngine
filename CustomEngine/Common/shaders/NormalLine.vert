/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: NormalLine.vert
Purpose: For render normal lines of central object
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/12/2019
End Header --------------------------------------------------------*/

#version 410 core

layout (std140) uniform Matrices
{
    mat4 projMatrix;
};
uniform mat4 modelMatrix;

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3  vPosition;

void main()
{
    vec4 vPos = vec4(vPosition, 1.0f);
    gl_Position = projMatrix * modelMatrix * vPos;
}
