/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Skybox.frag
Purpose: For render skybox
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/15/2019
End Header --------------------------------------------------------*/

#version 410 core
out vec3 color;

in vec3 TexCoords;

uniform samplerCube daySky;
uniform samplerCube nightSky;
uniform samplerCube dawnSky;
uniform vec3 skyRatio;

void main()
{
  vec3 daySky = skyRatio[0] * texture(daySky, TexCoords).rgb;
  vec3 dawnSky = skyRatio[1] * texture(dawnSky, TexCoords).rgb;
  vec3 nightSky = skyRatio[2] * texture(nightSky, TexCoords).rgb;
  
  color = daySky + dawnSky + nightSky;
  //color = vec3(1.f, 1.f, 1.f);
}