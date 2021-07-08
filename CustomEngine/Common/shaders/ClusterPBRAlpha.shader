Property
{
  float alpha = (0.5)
  float transmission = (1.0)
  float collimation = (0.0)
  float etaRatio = (1.0)
  color3 albedo = (0.8, 0.8, 0.8)
  color3 F0 = (0.04, 0.04, 0.04)
  float metallic = (0.0)
  float roughness = (1.0)
  float ao = (0.0)
  sampler2D diffMap = (6)
  sampler2D normMap = (7)
  sampler2D RMAMap = (8)
  bool haveNormMap = (false)
}

#VERT
#include ../Common/shaderlib/common.glsl
#include ../Common/shaderlib/bones.glsl
{
  // Input vertex data, different for all executions of this shader.
  layout(location = 0) in vec3 vPosition;
  layout(location = 1) in vec3 vNormal;
  layout(location = 2) in vec3 vTangent;
  layout(location = 3) in vec3 vBitangent;
  layout(location = 4) in vec2 vUV;
  layout(location = 5) in vec4 vBoneIDs;
  layout(location = 6) in vec4 vWeights;

  out VS_OUT
  {
    vec3 posWS;
    vec3 ModelNorm;
    vec2 UV;
    mat3 TBN;
    vec4 posCS;
  } vs_out;

  void main()
  {
    mat4 boneTransform = GetBoneTransform(vBoneIDs, vWeights);

    vec4 worldPos = modelMatrix * boneTransform * vec4(vPosition, 1.0f);

    vs_out.posWS = worldPos.xyz;
    vs_out.UV = vUV;
    vs_out.ModelNorm = (normalize(vNormal) + vec3(1.0f)) / 2.0f;

    vs_out.TBN = GetTBN(boneTransform, vNormal, vTangent, vBitangent);
    
    vs_out.posCS = finalMatrix * worldPos;
    
    gl_Position = vs_out.posCS;
  }
}

#FRAG
#include ../Common/shaderlib/mathutil.glsl
#include ../Common/shaderlib/common.glsl
#include ../Common/shaderlib/clustered.glsl
#include ../Common/shaderlib/lighting.glsl
#include ../Common/shaderlib/reflection.glsl
#include ../Common/shaderlib/pbr.glsl
#include ../Common/shaderlib/oit.glsl
{
  layout(location=0) out vec4 AccumOUT;
  layout(location=1) out vec4 BetaOUT;
  layout(location=2) out vec2 RefractOUT;
  in VS_OUT
  {
    vec3 posWS;
    vec3 ModelNorm;
    vec2 UV;
    mat3 TBN;
    vec4 posCS;
  } fs_in;

  vec3 colors[8] = vec3[]
  (
     vec3(0, 0, 0),    vec3(0,  0,  1), vec3(0, 1, 0),  vec3(0, 1,  1),
     vec3(1,  0,  0),  vec3(1,  0,  1), vec3(1, 1, 0),  vec3(1, 1, 1)
  );
  
  uniform vec2 bgSizeMeter;
  uniform samplerCube irradianceMap;
  uniform samplerCube prefilterMap;
  uniform sampler2D brdfLUT;
  uniform sampler2D depthTex;
  uniform vec3 sh[9];

  // Function prototypes
  vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float roughness, float metallic, vec3 F0);
  vec3 CalcDirLight(int index, vec3 worldPos, float clipPosZ, vec3 normal, vec3 viewDir, vec3 albedo, float roughness, float metallic, vec3 F0);
  
  void main()
  {
    //if(property.diffuse.a < 0.5) discard;
    vec4 color = vec4(0);
    
    vec3 normal;
    if (property.haveNormMap)
    {
      normal = NormalMapping(texture(property.normMap, fs_in.UV).rgb, fs_in.TBN);
    }
    else
    {
      normal = normalize(fs_in.TBN[2]);
    }
    vec3 normalCS = vec3(finalMatrix * vec4(normal, 0));
    
    vec4 diffTex = texture(property.diffMap, fs_in.UV).rgba;
    vec3 albedo = property.albedo * diffTex.rgb;
    vec3 rma = texture(property.RMAMap, fs_in.UV).rgb;
    rma.r *= property.roughness;
    rma.g *= property.metallic;
    rma.b *= property.ao;
    vec3 F0 = mix(property.F0, albedo, rma.g);

    // Locating which cluster you are a part of
    uint tileIndex = CurrentCluster(gl_FragCoord.xyz);

    // Point lights
    uint lightCount = lightGrid[tileIndex].count;
    uint lightIndexOffset = lightGrid[tileIndex].offset;

    // Reading from the global light list and calculating the radiance contribution of each light.
    vec3 lightCol = vec3(0.0);
    vec3 viewDir = GetViewDir(fs_in.posWS);

    // directional lights
    for (int i = 0; i < dirLightNum; i++)
      lightCol += CalcDirLight(i, fs_in.posWS, fs_in.posCS.z, normal, viewDir, albedo, rma.r, rma.g, F0);

    // point lights
    for (uint i = 0; i < lightCount; i++)
    {
      uint lightVectorIndex = globalLightIndexList[lightIndexOffset + i];
      //FragColor += vec4(pointLight[lightVectorIndex].color.rgb, 0.0);
      lightCol += calcPointLight(lightVectorIndex, normal, fs_in.posWS, viewDir, albedo, rma.r, rma.g, F0);
    }
    
    // IBL
    vec3 specular = vec3(0);
    AmbientIBL(color.rgb, specular, irradianceMap, prefilterMap, brdfLUT, albedo, normal, viewDir, F0, rma.r, rma.g, sh);
    
    color.a = property.alpha * diffTex.a;
    specular.rgb *= max(0,dot(normal, viewDir));
    // reflection
    //vec3 reflection = (1.0 - property.roughness) * ReflectCubemap(skybox, fs_in.posWS, normal, camPos);
    
    // OIT WEIGHT BLEND
    
    float alpha = color.a;
    vec3 transmitCol = albedo * property.transmission;
    
    // apply fog
    float S = FogAttenuation(fs_in.posWS);
    vec3 fog = mix(fogColor, vec3(0.0), S);
    
    // premultiplied emission
    vec3 emission = (min(color.rgb + lightCol + fog, vec3(1)) + specular) * alpha;
    
    WriteOIT(emission, alpha, transmitCol, property.collimation, 1 - property.etaRatio, fs_in.posCS.xyz, normalCS, bgSizeMeter * 10, depthTex, AccumOUT, BetaOUT, RefractOUT);
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