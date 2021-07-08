/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Depth.frag
Purpose: Render depth component of frag position
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_2
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 11/11/2019
End Header --------------------------------------------------------*/

#version 410 core

in vec2 TexCoords;

uniform sampler2D fbo_tex0;

// Output data
out vec3 color;

void main()
{
  color = texture(fbo_tex0, TexCoords).rrr;
}
