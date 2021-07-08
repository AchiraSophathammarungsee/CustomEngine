/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Phong.frag
Purpose: Render basic Phong lighting model + multiple lights support
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/14/2019
End Header --------------------------------------------------------*/

#version 410 core

uniform vec3 color;

// Output data
out vec3 outColor;

void main()
{ 
	outColor = color;
}
