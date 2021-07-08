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

#version 430 core

struct VolumeTileAABB
{
    vec4 minPoint;
    vec4 maxPoint;
};

layout (std140) uniform Matrices
{
  mat4 projMatrix;
};

layout (std430, binding = 1) buffer clusterAABB
{
    VolumeTileAABB cluster[ ];
};
layout (std430, binding = 2) buffer screenToView
{
    mat4 inverseProjection;
    uvec4 tileSizes;
    uvec2 screenDimensions;
};

uniform int clusterNum;

vec4 clipToView(vec4 clip)
{
  // View space transform
  vec4 view = inverseProjection * clip;

  // Perspective projection
  view = view / view.w;
    
  return view;
}

vec4 screen2View(vec4 screen)
{
  // Convert to NDC
  vec2 texCoord = screen.xy / screenDimensions.xy;

  // Convert to clipSpace
  // vec4 clip = vec4(vec2(texCoord.x, 1.0 - texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
  vec4 clip = vec4(vec2(texCoord.x, texCoord.y) * 2.0 - 1.0, screen.z, screen.w);

  // Not sure which of the two it is just yet

  return clipToView(clip);
}

// return 1 if point is inside, 0 other wise
float pointAABB(vec4 point, int clusterID)
{
  // convert cluster's min and max (view space) into NDC using projection matrix
  vec4 min = projMatrix * cluster[clusterID].minPoint;
  vec4 max = projMatrix * cluster[clusterID].maxPoint;

  vec3 s = step(min.xyz, point.xyz) - step(max.xyz, point.xyz);

  return s.x * s.y * s.z; 
}

const vec3 clrList[8] = vec3[8]( vec3(0.8), vec3(0.2), vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 1.0), vec3(0.0, 1.0, 1.0) );

out vec3 color;

uniform float zNear;
uniform float zFar;
float linearDepth(float depthSample)
{
    float depthRange = 2.0 * depthSample - 1.0;
    // Near... Far... wherever you are...
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    // float linear = 2.0; 
    return linear;
}

void main()
{
  // clusters are in view space
  // convert frag coord (screen space) back to view space
  // vec3 viewFrag = screen2View(gl_FragCoord).xyz;

  //Locating which cluster you are a part of
  uint zTile     = uint(max(log2(linearDepth(gl_FragCoord.z)), 0));
  uvec3 tiles    = uvec3( uvec2( gl_FragCoord.xy / tileSizes[3] ), zTile);
  uint tileIndex = tiles.x +
                   tileSizes.x * tiles.y +
                   (tileSizes.x * tileSizes.y) * tiles.z;
  
  int colorID = int(tileIndex - (8 * floor(tileIndex/8)));
  color = clrList[colorID];
  return;
}
