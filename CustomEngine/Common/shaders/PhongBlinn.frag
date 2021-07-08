/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: PhongBlinn.frag
Purpose: Modify phong lighting to PhongBlinn model.
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
    bool lit;
}; 
uniform Material material;

struct DirLight
{
  vec3 direction;
  vec3 color;
};
#define DIR_LIGHTS_NUM 1
uniform DirLight dirLights[DIR_LIGHTS_NUM];

struct PointLight
{
  vec3 position;
  vec3 color;
  
  float constant;
  float linear;
  float quadratic;
};
#define POINT_LIGHTS_NUM 4
uniform PointLight pointLights[POINT_LIGHTS_NUM];

struct SpotLight
{
  vec3 position;
  vec3 direction;
  vec3 color;
  
  float innerOff;
  float outerOff;
};
#define SPOT_LIGHTS_NUM 1
uniform SpotLight spotLights[SPOT_LIGHTS_NUM];

uniform vec3 camPos;

in VS_OUT
{
  vec3 Normal;
  vec3 FragPos;
} fs_in;

// Output data
out vec3 color;


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.direction);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  
  // combine results
  vec3 ambient  = light.color * material.ambient;
  vec3 diffuse  = light.color * diff * material.diffuse;
  vec3 specular = light.color * spec * material.specular;
  return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));
             
    // combine results
    vec3 ambient  = light.color * material.ambient;
    vec3 diffuse  = light.color * diff * material.diffuse;
    vec3 specular = light.color * spec * material.specular;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(light.direction);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  
  float theta = dot(lightDir, normalize(-light.direction));
    
  if(theta > light.outerOff) 
  {
    // calculate falloff intensity at current fragment
    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.innerOff - light.outerOff;
    float intensity = clamp((theta - light.outerOff) / epsilon, 0.0, 1.0);    
  
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // combine results
    vec3 ambient  = light.color * material.ambient;
    vec3 diffuse  = light.color * diff * material.diffuse * intensity;
    vec3 specular = light.color * spec * material.specular * intensity;
    return (ambient + diffuse + specular);
  }
  else  // else, use ambient light so scene isn't completely dark outside the spotlight.
    return material.ambient;
}

void main()
{
  if(!material.lit)
  {
    color = material.diffuse;
    return;
  }
  
  vec3 normal = normalize(fs_in.Normal);
  vec3 viewDir = normalize(camPos - fs_in.FragPos);

  vec3 result;
  
  for(int i = 0; i < DIR_LIGHTS_NUM; i++)
    result += CalcDirLight(dirLights[i], normal, viewDir);
  
  for(int i = 0; i < POINT_LIGHTS_NUM; i++)
    result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir);
    
  for(int i = 0; i < SPOT_LIGHTS_NUM; i++)
    result += CalcSpotLight(spotLights[i], normal, viewDir);
  
	color = result;
}
