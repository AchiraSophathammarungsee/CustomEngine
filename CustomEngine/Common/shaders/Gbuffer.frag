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

#version 410 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAmbient;
layout (location = 3) out vec4 gDiffuse;
layout (location = 4) out vec4 gSpec;

in VS_OUT 
{
  vec3 ModelPos;
  vec3 ModelNorm;
  vec2 UV;
  mat3 TBN;
} fs_in;

struct Material {
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

void main()
{    
    vec2 UVcoord = CalculateUV();

    gPosition = fs_in.ModelPos;
    
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
    gNormal = normal;

    vec3 diffTex = material.diffuse;
    if(material.haveDiffMap)
      diffTex *= texture(material.diffMap, UVcoord).rgb;
    gDiffuse.rgb = diffTex;
    gDiffuse.a = 1.0f;

    gSpec.rgb = material.specular * texture(material.specMap, UVcoord).r;
    gSpec.a = material.shininess;

    gAmbient = material.ambient;
}  
