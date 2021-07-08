Property
{
}

#VERT
#include ../Common/shaderlib/common.glsl
#include ../Common/shaderlib/bones.glsl
{
  layout(location = 0) in vec3  vPosition;
  layout(location = 1) in vec3  vNormal;
  layout(location = 2) in vec3  vTangent;
  layout(location = 3) in vec3  vBitangent;
  layout(location = 4) in vec2  vUV;

  out vec2 TexCoords;

  void main()
  {
      TexCoords = vUV;
      gl_Position = vec4(2.f * vPosition, 1.0f);
  }
}

#FRAG
#include ../Common/shaderlib/mathutil.glsl
#include ../Common/shaderlib/common.glsl
#include ../Common/shaderlib/clustered.glsl
#include ../Common/shaderlib/lighting.glsl
#include ../Common/shaderlib/pbr.glsl
{
  out vec3 FragColor;
  
  uniform sampler2D fbo_tex0;
  uniform sampler2D fbo_tex1;
  uniform sampler2D fbo_tex2;
  uniform sampler2D fbo_tex3;
  uniform sampler2D fbo_tex4;
  uniform sampler2D ssao;
  uniform samplerCube irradianceMap;
  uniform samplerCube prefilterMap;
  uniform sampler2D brdfLUT;
  uniform vec3 sh[9];

  in vec2 TexCoords;

  // Function prototypes
  vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float roughness, float metallic, vec3 F0);
  vec3 CalcDirLight(int index, vec3 worldPos, float clipPosZ, vec3 normal, vec3 viewDir, vec3 albedo, float roughness, float metallic, vec3 F0);
  
  void main()
  {
    vec3 WorldPos = texture(fbo_tex0, TexCoords).rgb;
    vec3 Normal = texture(fbo_tex1, TexCoords).rgb;
    vec3 Albedo = texture(fbo_tex2, TexCoords).rgb;
    vec3 RMA = texture(fbo_tex3, TexCoords).rgb;
    vec3 F0 = texture(fbo_tex4, TexCoords).rgb;

    // Locating which cluster you are a part of
    vec4 clipSpacePos = finalMatrix * vec4(WorldPos, 1);
    vec3 ndcCoord = clipSpacePos.xyz / clipSpacePos.w;
    uint tileIndex = CurrentCluster((ndcCoord + 1.0) * 0.5 * vec3(screenParam, 1));

    // Point lights
    uint lightCount = lightGrid[tileIndex].count;
    uint lightIndexOffset = lightGrid[tileIndex].offset;

    // Reading from the global light list and calculating the radiance contribution of each light.
    vec3 lightCol = vec3(0.0);
    vec3 viewDir = GetViewDir(WorldPos);

    // directional lights
    for (int i = 0; i < dirLightNum; i++)
      lightCol.rgb += CalcDirLight(i, WorldPos, clipSpacePos.z, Normal, viewDir, Albedo, RMA.r, RMA.g, F0);
    
    // point lights
    for (uint i = 0; i < lightCount; i++)
    {
      uint lightVectorIndex = globalLightIndexList[lightIndexOffset + i];
      //FragColor += pointLight[lightVectorIndex].color.rgb;
      lightCol.rgb += calcPointLight(lightVectorIndex, Normal, WorldPos, viewDir, Albedo, RMA.r, RMA.g, F0);
    }
    
    // IBL
    vec3 ambient, specular;
    AmbientIBL(ambient, specular, irradianceMap, prefilterMap, brdfLUT, Albedo, Normal, viewDir, F0, RMA.r, RMA.g, sh);
    ambient += specular;
    
    // global ambient * albedo * AO
    //ambient *= Albedo * RMA.b;
    FragColor.rgb = ambient + lightCol;
    FragColor.rgb *= texture(ssao, TexCoords).rrr * RMA.b;

    // apply fog
    float S = FogAttenuation(WorldPos);
    FragColor = mix(fogColor, FragColor, S);
  }

  vec3 CalcDirLight(int index, vec3 worldPos, float clipPosZ, vec3 normal, vec3 viewDir, vec3 albedo, float roughness, float metallic, vec3 F0)
  {
    vec3 lightDir = normalize(-dirLights[index].direction.xyz);
    // combine results
    vec3 lightColor = dirLights[index].color.rgb;
    
    vec3 radiance = DirLightReflectanceEq(normal, viewDir, lightDir, lightColor, albedo, roughness, metallic, F0);

    // calculate shadow
    vec3 shadow = DirLightShadow(index, worldPos, clipPosZ, normal);
    radiance *= shadow;

    return radiance;
  }
  
  float pow2(float val)
  {
    return val * val;
  }

  vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float roughness, float metallic, vec3 F0)
  {
    //Point light basics
    vec3 position = pointLight[index].position.xyz;
    vec3 color = pointLight[index].intensity * pointLight[index].color.rgb;
    float radius = pointLight[index].range;

    //Attenuation calculation that is applied to all
    float distance = length(position - fragPos);
    float attenuation = pow2(clamp(1 - (distance / radius), 0.0, 1.0)) / (1.0 + (distance * distance));
    
    vec3 radiance = PointLightReflectanceEq(fragPos, normal, viewDir, position, color, attenuation, albedo, roughness, metallic, F0);
    
    //float shadow = PointLightShadow(index, fragPos);
    //radiance *= shadow;

    return radiance;
  }
}