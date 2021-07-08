/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: DepthMap.frag
Purpose: For render light's shadow depth map
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/15/2019
End Header --------------------------------------------------------*/

#version 410 core

layout (location = 0) out vec4 atlasTexture;

uniform sampler2D tex;

in vec2 UV;

void main()
{             
    // gl_FragDepth = gl_FragCoord.z;
    atlasTexture = texture(tex, UV);
}
