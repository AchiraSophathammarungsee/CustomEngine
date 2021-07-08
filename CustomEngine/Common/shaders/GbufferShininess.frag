/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: GbufferShininess.frag
Purpose: Displays Shininess texture
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS350_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 1/21/2020
End Header --------------------------------------------------------*/

#version 410 core

uniform sampler2D fbo_tex0;
uniform sampler2D fbo_tex1;
uniform sampler2D fbo_tex2;
uniform sampler2D fbo_tex3;
uniform sampler2D fbo_tex4;

in vec2 TexCoords;

// Output data
out vec3 color;

void main()
{
  color = vec3(texture(fbo_tex4, TexCoords).a);
}
