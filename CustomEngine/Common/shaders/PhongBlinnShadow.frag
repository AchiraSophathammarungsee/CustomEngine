/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: PhongBlinnShadow.frag
Purpose: PhoneBlinn lighting model + shadow map support
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/15/2019
End Header --------------------------------------------------------*/

#version 410 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float reflection;
    
    bool haveDiffMap;
    bool haveNormMap;
    bool haveSpecMap;
    sampler2D diffMap;
    sampler2D normMap;
    sampler2D specMap;
    
    bool lit;
}; 
uniform Material material;

struct UVwrap {
  int type;
  bool texEntityPos; 
  
  vec3 BBcenter;
  vec3 BBmax;
  vec3 BBmin;
  
  vec2 UVscale;
  vec2 UVshift;
};
#define UV_TYPE_VERTEX 0
#define UV_TYPE_PLANAR 1
#define UV_TYPE_SPHERE 2
#define UV_TYPE_CYLINDER 3
uniform UVwrap uvwrap;

struct DirLight
{
  vec3 direction;
  vec3 diffuse;
  vec3 ambient;
  vec3 specular;
};
#define DIR_LIGHTS_NUM 16
uniform DirLight dirLights[DIR_LIGHTS_NUM];
uniform int dirLightNum;

struct PointLight
{
  vec3 position;
  vec3 diffuse;
  vec3 ambient;
  vec3 specular;
  
  float constant;
  float linear;
  float quadratic;
};
#define POINT_LIGHTS_NUM 16
uniform PointLight pointLights[POINT_LIGHTS_NUM];
uniform int pointLightNum;

struct SpotLight
{
  vec3 position;
  vec3 direction;
  vec3 diffuse;
  vec3 ambient;
  vec3 specular;
  
  float innerOff;
  float outerOff;
  float constant;
  float linear;
  float quadratic;
};
#define SPOT_LIGHTS_NUM 16
uniform SpotLight spotLights[SPOT_LIGHTS_NUM];
uniform int spotLightNum;

uniform vec3 camPos;

in VS_OUT
{
  vec3 ModelPos;
  vec3 ModelNorm;
  vec2 UV;
  vec3 FragPos;
  vec4 FragPosLightSpace;
  mat3 TBN;
} fs_in;

uniform sampler2D shadowMap;

uniform samplerCube daySky;
uniform samplerCube nightSky;
uniform samplerCube dawnSky;
uniform vec3 skyRatio;

vec3 diffTexture;
vec3 normTexture;
vec3 specTexture;

uniform vec2 farPlane;
uniform vec3 fogColor;
uniform vec3 globalAmbient;

// Output data
out vec3 color;

vec2 CalculatePlanar(vec3 entity)
{
  vec3 absVec= abs(entity);
  vec2 uv = vec2(0.f);
  
  if(absVec.x >= absVec.y && absVec.x >= absVec.z)
  {
    uv.x = (entity.x < 0.0f) ? entity.z : -entity.z;
    uv.y = entity.y;
  }
  else if(absVec.y >= absVec.x && absVec.y >= absVec.z)
  {
    uv.x = (entity.y < 0.0f) ? entity.x : -entity.x;
    uv.y = entity.z;
  }
  else
  {
    uv.x = (entity.z < 0.0f) ? entity.x : -entity.x;
    uv.y = entity.y;
  }
  
  return (uv + vec2(1.0f, 1.0f)) / 2.0f;
}

#define M_PI 3.1415926535897932384626433832795
vec2 CalculateSphere(vec3 entity)
{
  entity -= uvwrap.BBcenter;
  
  float r = sqrt(entity.x*entity.x + entity.y*entity.y + entity.z*entity.z);
  float theta = atan(entity.y, entity.x);
  float phi = acos(entity.z / r);
  
  return vec2(theta/(2 * M_PI), (M_PI - phi)/M_PI);
}

vec2 CalculateCylinder(vec3 entity)
{
  entity -= uvwrap.BBcenter;

  float theta = atan(entity.y, entity.x);
  float z = (entity.z - uvwrap.BBmin.z) / (uvwrap.BBmax.z - uvwrap.BBmin.z);
  
  return vec2(theta / (2 * M_PI), z);
}

vec2 CalculateUV()
{
  vec3 entity = (uvwrap.texEntityPos) ? fs_in.ModelPos : fs_in.ModelNorm;
  
  if(uvwrap.type == UV_TYPE_VERTEX)
  {
    return fs_in.UV * uvwrap.UVscale + uvwrap.UVshift;
  }
  else if(uvwrap.type == UV_TYPE_PLANAR)
  {
    return CalculatePlanar(entity) * uvwrap.UVscale + uvwrap.UVshift;
  }
  else if(uvwrap.type == UV_TYPE_SPHERE)
  {
    return CalculateSphere(entity) * uvwrap.UVscale + uvwrap.UVshift;
  }
  else if(uvwrap.type == UV_TYPE_CYLINDER)
  {
    return CalculateCylinder(entity) * uvwrap.UVscale + uvwrap.UVshift;
  }
  return vec2(0.f, 0.f);
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;
  float closestDepth = texture(shadowMap, projCoords.xy).r;
  float currentDepth = projCoords.z;
  float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
  //float bias = 0.005f;
  
  // PCF
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
  for(int x = -1; x <= 1; ++x)
  {
    for(int y = -1; y <= 1; ++y)
    {
      float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
      shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
    }
  }
  shadow /= 9.0;
  
  if(projCoords.z > 1.0)
    shadow = 0.0;
  
  return shadow;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.direction);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  
  // combine results
  vec3 ambient  = light.ambient * material.ambient * diffTexture;
  vec3 diffuse  = light.diffuse * diff * material.diffuse * diffTexture;
  vec3 specular = light.specular * spec * material.specular * specTexture;
  
  // calculate shadow
  float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);
  vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));
  return lighting;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position - fragPos);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  // attenuation
  float distance    = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
             
  // combine results
  vec3 ambient  = light.ambient * material.ambient * diffTexture;
  vec3 diffuse  = light.diffuse * diff * material.diffuse * diffTexture;
  vec3 specular = light.specular * spec * material.specular * specTexture;
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;
  return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position - fragPos);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  
  // ambient
  vec3 ambient = light.ambient * material.ambient * diffTexture;
   
  // diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * material.diffuse * diffTexture;  
    
  // specular
  float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);
  vec3 specular = light.specular * spec * material.specular * specTexture;  
    
  // spotlight (soft edges)
  float theta = dot(lightDir, normalize(-light.direction)); 
  float epsilon = (light.innerOff - light.outerOff);
  float intensity = clamp((theta - light.outerOff) / epsilon, 0.0, 1.0);
  diffuse  *= intensity;
  specular *= intensity;
    
  // attenuation
  float distance    = length(light.position - fragPos);
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
  vec2 UVcoord = CalculateUV();
  diffTexture = (material.haveDiffMap) ? texture(material.diffMap, UVcoord).rgb : vec3(1.f);
  specTexture = (material.haveSpecMap) ? texture(material.specMap, UVcoord).rgb : vec3(1.f);
  
  if(!material.lit)
  {
    color = material.diffuse * diffTexture;
    return;
  }
  
  vec3 normal;
  if(material.haveNormMap)
  {
    normal = texture(material.normMap, UVcoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(fs_in.TBN * normal);
  }
  else
  {
    normal = normalize(fs_in.TBN[2]);
  }
  
  vec3 viewDir = normalize(camPos - fs_in.FragPos);

  vec3 result = globalAmbient * material.ambient;
  
  for(int i = 0; i < dirLightNum; i++)
    result += CalcDirLight(dirLights[i], normal, viewDir);
  
  for(int i = 0; i < pointLightNum; i++)
    result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir);
    
  for(int i = 0; i < spotLightNum; i++)
    result += CalcSpotLight(spotLights[i], normal, fs_in.FragPos, viewDir);
  
  vec3 I = normalize(fs_in.FragPos - camPos);
  vec3 R = GetReflection(I, normal);
  vec3 daySky = skyRatio[0] * texture(daySky, R).rgb;
  vec3 dawnSky = skyRatio[1] * texture(dawnSky, R).rgb;
  vec3 nightSky = skyRatio[2] * texture(nightSky, R).rgb;
  vec3 reflectCol = daySky + dawnSky + nightSky;
  
  result =(1.0f - material.reflection) * result + material.reflection * reflectCol;
  
  float S = FogAttenuation(camPos - fs_in.FragPos);
  color = S*result + (1.0f - S)*fogColor;
}
