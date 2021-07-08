/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Lightpass.frag
Purpose: Calculate lighting from g-buffers
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

struct DirLight
{
  vec4 direction;
  vec4 diffuse;
  vec4 ambient;
  vec4 specular;

  mat4 lightSpaceMatrix;
  int castShadow;
};
#define DIR_LIGHTS_NUM 16

struct PointLight
{
  vec4 position;
  vec4 diffuse;
  vec4 ambient;
  vec4 specular;
  float constant;
  float linear;
  float quadratic;
  float squaredRadius;
};
#define POINT_LIGHTS_NUM 16

struct SpotLight
{
  vec4 position;
  vec4 direction;
  vec4 diffuse;
  vec4 ambient;
  vec4 specular;
  
  float innerOff;
  float outerOff;
  float constant;
  float linear;
  float quadratic;

  mat4 lightSpaceMatrix;
  float farPlane;
  float nearPlane;
  int castShadow;
};
#define SPOT_LIGHTS_NUM 16

layout (std140) uniform Lights 
{
  int dirLightNum;
  int pointLightNum;
  int spotLightNum;
  int shadowMapTilesPerRow;
  DirLight dirLights[DIR_LIGHTS_NUM];
  PointLight pointLights[POINT_LIGHTS_NUM];
  SpotLight spotLights[SPOT_LIGHTS_NUM];
};

uniform sampler2D shadowMaps;

vec3 FragPos;
vec3 Normal;
vec3 Ambient;
vec3 Diffuse;
vec3 Specular;
float Shininess;
float ShadowMap;

uniform vec2 farPlane;
uniform vec3 fogColor;
uniform vec3 globalAmbient;

uniform vec3 camPos;

in vec2 TexCoords;

// Output data
out vec3 color;

float LinearizeDepth(float depth, int index)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * spotLights[index].nearPlane * spotLights[index].farPlane) / (spotLights[index].farPlane + spotLights[index].nearPlane - z * (spotLights[index].farPlane - spotLights[index].nearPlane));
}

float ShadowCalculation(int index, vec4 fragPosLightSpace, vec3 normal)
{
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;

  float tileSize = 1.f / shadowMapTilesPerRow;
  projCoords = projCoords * vec3(tileSize,tileSize,1.0) + vec3((index % shadowMapTilesPerRow) * tileSize, (index / shadowMapTilesPerRow) * tileSize, 0.f);

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
  float spec = pow(max(dot(normal, halfwayDir), 0.0), Shininess);
  
  // combine results
  vec3 ambient  = dirLights[index].ambient.xyz * Ambient;
  vec3 diffuse  = dirLights[index].diffuse.xyz * diff * Diffuse;
  vec3 specular = dirLights[index].specular.xyz * spec * Specular;
  
  // calculate shadow
  if(dirLights[index].castShadow > 0)
  {
    float shadow = ShadowCalculation(index, dirLights[index].lightSpaceMatrix * vec4(FragPos, 1.0), normal);
    return (ambient + (1.0 - shadow) * (diffuse + specular));
  }
  
  return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position.xyz - fragPos);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  float spec = pow(max(dot(normal, halfwayDir), 0.0), Shininess);
  // attenuation
  float distance    = length(light.position.xyz - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
             
  // combine results
  vec3 ambient  = light.ambient.xyz * Ambient;
  vec3 diffuse  = light.diffuse.xyz * diff * Diffuse;
  vec3 specular = light.specular.xyz * spec * Specular;
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;
  return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position.xyz - fragPos);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  
  // ambient
  vec3 ambient = light.ambient.xyz * Ambient;
   
  // diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = light.diffuse.xyz * diff * Diffuse;  
    
  // specular
  float spec = pow(max(dot(viewDir, halfwayDir), 0.0),Shininess);
  vec3 specular = light.specular.xyz * spec * Specular;  
    
  // spotlight (soft edges)
  float theta = dot(lightDir, normalize(-light.direction.xyz)); 
  float epsilon = (light.innerOff - light.outerOff);
  float intensity = clamp((theta - light.outerOff) / epsilon, 0.0, 1.0);
  diffuse  *= intensity;
  specular *= intensity;
    
  // attenuation
  float distance    = length(light.position.xyz - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
  ambient  *= attenuation; 
  diffuse   *= attenuation;
  specular *= attenuation;   
        
  return (ambient + diffuse + specular);
}

vec3 GetReflection(vec3 a, vec3 b)
{
  return a - 2.0f * dot(b, a) * b;
}

float FogAttenuation(vec3 viewVec)
{
  return (farPlane.y - length(viewVec)) / (farPlane.y - farPlane.x);
}

void main()
{
  FragPos = texture(fbo_tex0, TexCoords).rgb;
  Normal = texture(fbo_tex1, TexCoords).rgb;
  Ambient = texture(fbo_tex2, TexCoords).rgb;
  Diffuse = texture(fbo_tex3, TexCoords).rgb;
  Specular = texture(fbo_tex4, TexCoords).rgb;
  Shininess = texture(fbo_tex4, TexCoords).a;
  ShadowMap = texture(fbo_tex3, TexCoords).a;

  vec3 viewDir = normalize(camPos - FragPos);

  vec3 result = globalAmbient * Ambient;
  
  for(int i = 0; i < dirLightNum; i++)
    result += CalcDirLight(i, Normal, viewDir);
  
  for(int i = 0; i < pointLightNum; i++)
  {
    vec3 diff = pointLights[i].position.xyz - FragPos;
	  float squaredDistance = (diff.x*diff.x + diff.y*diff.y + diff.z*diff.z);
    if(squaredDistance < pointLights[i].squaredRadius * 1.0f)
    {
      result += CalcPointLight(pointLights[i], Normal, FragPos, viewDir);
    }
  }
    
  for(int i = 0; i < spotLightNum; i++)
    result += CalcSpotLight(spotLights[i], Normal, FragPos, viewDir);
  
  float S = FogAttenuation(camPos - FragPos);
  color = S*result + (1.0f - S)*fogColor;
}
