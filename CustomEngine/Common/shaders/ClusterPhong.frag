/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Gbuffer.frag
Purpose: outputs each geomtry informations onto textures
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS350_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 1/21/2020
End Header --------------------------------------------------------*/

#version 430 core

#define M_PI 3.1415926535897932384626433832795

out vec4 FragColor;

in VS_OUT 
{
  vec3 posWS;
  vec3 ModelNorm;
  vec2 UV;
  mat3 TBN;
} fs_in;

struct Material 
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;

  bool haveDiffMap;
  bool haveNormMap;
  bool haveSpecMap;
  sampler2D diffMap;
  sampler2D normMap;
  sampler2D specMap;
}; 
uniform Material material;

// Cluster shading structs and buffers
struct PointLight
{
  vec4 position;
  vec4 color;
  bool enabled;
  float intensity;
  float range;
};
struct LightGrid
{
    uint offset;
    uint count;
};
struct VolumeTileAABB
{
  vec4 minPoint;
  vec4 maxPoint;
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
  float scale;
  float bias;
};

layout (std430, binding = 3) buffer lightSSBO
{
  PointLight pointLight[];
};
layout (std430, binding = 4) buffer lightIndexSSBO
{
  uint globalLightIndexList[];
};
layout (std430, binding = 5) buffer lightGridSSBO
{
  LightGrid lightGrid[];
};

struct DirLight
{
  vec4 direction;
  vec4 color;

  mat4 lightSpaceMatrix;
  int castShadow;
};
#define DIR_LIGHTS_NUM 16

layout (std140, binding = 7) uniform dirLightsUBO 
{
  int dirLightNum;                     // 4
  int shadowMapTilesPerRow;            // 4
  DirLight dirLights[DIR_LIGHTS_NUM];  // 100 * DIR_LIGHTS_NUM
};

uniform sampler2D shadowMaps;

vec3 colors[8] = vec3[]
(
   vec3(0, 0, 0),    vec3( 0,  0,  1), vec3( 0, 1, 0),  vec3(0, 1,  1),
   vec3(1,  0,  0),  vec3( 1,  0,  1), vec3( 1, 1, 0),  vec3(1, 1, 1)
);

uniform float zFar;
uniform float zNear;
uniform vec3 cameraPos_wS;

uniform vec2 farPlane;
uniform vec3 fogColor;

// Function prototypes
float linearDepth(float depthSample);
vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo);
vec3 CalcDirLight(int index, vec3 normal, vec3 viewDir);
float FogAttenuation(vec3 viewVec);

void main()
{ 
  // alpha clipping
  //if(material.ambient.a < 0.5) discard;
    
  vec3 normal;
  if(material.haveNormMap)
  {
    normal = texture(material.normMap, fs_in.UV).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(fs_in.TBN * normal);
  }
  else
  {
    normal = normalize(fs_in.TBN[2]);
  }

  vec3 diffTex = material.diffuse;
  if(material.haveDiffMap)
    diffTex *= texture(material.diffMap, fs_in.UV).rgb;
  
  vec3 spec = material.specular * texture(material.specMap, fs_in.UV).r;
  material.shininess;

  material.ambient;

  // Locating which cluster you are a part of
  uint zTile     = uint(max(log2(linearDepth(gl_FragCoord.z)) * scale + bias, 0.0));
  uvec3 tiles    = uvec3( uvec2( gl_FragCoord.xy / tileSizes[3] ), zTile);
  uint tileIndex = tiles.x +
                   tileSizes.x * tiles.y +
                   (tileSizes.x * tileSizes.y) * tiles.z;

  //uint clrID = uint(tileIndex - (7 * floor(tileIndex/7)));
  //FragColor = vec4(colors[clrID], 1.0);
  //return;

  // Point lights
  uint lightCount       = lightGrid[tileIndex].count;
  uint lightIndexOffset = lightGrid[tileIndex].offset;

  // Reading from the global light list and calculating the radiance contribution of each light.
  FragColor = vec4(0.0, 0.0, 0.0, 1.0);
  vec3 viewDir = normalize(cameraPos_wS - fs_in.posWS);

  // directional lights
  for(int i = 0; i < dirLightNum; i++)
    FragColor += vec4(CalcDirLight(i, normal, viewDir), 0.0);
  
  // point lights
  for(uint i = 0; i < lightCount; i++)
  {
    uint lightVectorIndex = globalLightIndexList[lightIndexOffset + i];
    //FragColor += vec4(pointLight[lightVectorIndex].color.rgb, 0.0);
    FragColor += vec4(calcPointLight(lightVectorIndex, normal, fs_in.posWS, viewDir, diffTex), 0.0);
  }

  // apply fog
  float S = FogAttenuation(cameraPos_wS - fs_in.posWS);
  FragColor = mix(vec4(fogColor, 1.0), FragColor, S);
}  

float linearDepth(float depthSample)
{
  float depthRange = 2.0 * depthSample - 1.0;
  // Near... Far... wherever you are...
  float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
  return linear;
}

float ShadowCalculation(int index, vec4 fragPosLightSpace, vec3 normal)
{
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

  projCoords = projCoords * 0.5 + 0.5;

  if(projCoords.x <= 0 || projCoords.x >= 1 || projCoords.y <= 0 || projCoords.y >= 1 ) return 0.0;

  float tileSize = 1.f / shadowMapTilesPerRow;
  int xTile = (index % shadowMapTilesPerRow);
  int yTile = (index / shadowMapTilesPerRow);
  vec3 offset = vec3(xTile * tileSize, yTile * tileSize, 0.f);

  projCoords = projCoords * vec3(tileSize,tileSize,1.0) + offset;

  float closestDepth = texture(shadowMaps, projCoords.xy).r;
  //closestDepth = vec3(LinearizeDepth(depthValue) / far_plane;
  float currentDepth = projCoords.z;
  float bias = max(0.05 * (1.0 - dot(normal, dirLights[index].direction.xyz)), 0.005);
  //float bias = 0.005f;
  
  // PCF
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMaps, 0);
  for(int x = -1; x <= 1; ++x)
  {
    for(int y = -1; y <= 1; ++y)
    {
      float pcfDepth = texture(shadowMaps, projCoords.xy + vec2(x, y) * texelSize).r; 
      shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
    }
  }
  shadow /= 9.0;
  
  if(projCoords.z > 1.0)
    shadow = 0.0;
  
  return shadow;
}

vec3 CalcDirLight(int index, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-dirLights[index].direction.xyz);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  
  // combine results
  vec3 diffuse  = dirLights[index].color.rgb * diff * material.diffuse;
  vec3 specular = dirLights[index].color.rgb * spec;
  
  // calculate shadow
  if(dirLights[index].castShadow > 0)
  {
    float shadow = ShadowCalculation(index, dirLights[index].lightSpaceMatrix * vec4(fs_in.posWS, 1.0), normal);
    return (1.0 - shadow) * (diffuse + specular);
  }
  
  return diffuse + specular;
}

vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos,
                    vec3 viewDir, vec3 albedo)
{
    //Point light basics
    vec3 position = pointLight[index].position.xyz;
    vec3 color    = 100.0 * pointLight[index].color.rgb;
    float radius  = pointLight[index].range;

    //Stuff common to the BRDF subfunctions 
    vec3 lightDir = normalize(position - fragPos);
    vec3 halfway  = normalize(lightDir + viewDir);
    float nDotV = max(dot(normal, viewDir), 0.0);
    float nDotL = max(dot(normal, lightDir), 0.0);

    //Attenuation calculation that is applied to all
    float distance    = length(position - fragPos);
    float attenuation = pow(clamp(1 - pow((distance / radius), 4.0), 0.0, 1.0), 2.0)/(1.0  + (distance * distance) );
    vec3 radianceIn   = color * attenuation;

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 radiance = radianceIn * nDotL;// * ((material.diffuse / M_PI) + spec );

    return radiance;
}

float FogAttenuation(vec3 viewVec)
{
  return (farPlane.y - length(viewVec)) / (farPlane.y - farPlane.x);
}