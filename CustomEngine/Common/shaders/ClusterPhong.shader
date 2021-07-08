Property
{
  color3 ambient = (0.1,0.1,0.1)
  color3 diffuse = (0.8, 0.8, 0.8)
  color3 specular = (1, 1, 1)
  float shininess = (32.0)
  bool haveDiffMap = (false)
  bool haveNormMap = (false)
  bool haveSpecMap = (false)
  sampler2D diffMap = (2)
  sampler2D normMap = (3)
  sampler2D specMap = (4)
  float sssDistort = (1)
  float sssScale = (1)
  float sssPower = (1)
  float sssAtten = (1)
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
  } vs_out;

  void main()
  {
    mat4 boneTransform = GetBoneTransform(vBoneIDs, vWeights);

    vec4 worldPos = modelMatrix * boneTransform * vec4(vPosition, 1.0f);

    vs_out.posWS = worldPos.xyz;
    vs_out.UV = vUV;
    vs_out.ModelNorm = (normalize(vNormal) + vec3(1.0f)) / 2.0f;

    vs_out.TBN = GetTBN(boneTransform, vNormal, vTangent, vBitangent);

    gl_Position = finalMatrix * worldPos;
  }
}

#FRAG
#include ../Common/shaderlib/common.glsl
#include ../Common/shaderlib/clustered.glsl
#include ../Common/shaderlib/lighting.glsl
{
  out vec4 FragColor;

  in VS_OUT
  {
    vec3 posWS;
    vec3 ModelNorm;
    vec2 UV;
    mat3 TBN;
  } fs_in;

  vec3 colors[8] = vec3[]
  (
     vec3(0, 0, 0),    vec3(0,  0,  1), vec3(0, 1, 0),  vec3(0, 1,  1),
     vec3(1,  0,  0),  vec3(1,  0,  1), vec3(1, 1, 0),  vec3(1, 1, 1)
  );

  // Function prototypes
  vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo);
  vec3 CalcDirLight(int index, vec3 normal, vec3 viewDir);

  void main()
  {
    // alpha clipping
    //if(property.ambient.a < 0.5) discard;

    vec3 normal;
    if (property.haveNormMap)
    {
      normal = NormalMapping(texture(property.normMap, fs_in.UV).rgb, fs_in.TBN);
    }
    else
    {
      normal = normalize(fs_in.TBN[2]);
    }

    vec3 diffTex = property.diffuse;
    if (property.haveDiffMap)
      diffTex *= texture(property.diffMap, fs_in.UV).rgb;

    //vec3 spec = property.specular;
    //if(property.haveSpecMap)
    //  spec *= texture(property.specMap, fs_in.UV).r;
    //property.shininess;
    //property.ambient;

    // Locating which cluster you are a part of
    uint tileIndex = CurrentCluster(gl_FragCoord.xyz);

    //uint clrID = uint(tileIndex - (7 * floor(tileIndex/7)));
    //FragColor = vec4(colors[clrID], 1.0);
    //return;

    // Point lights
    uint lightCount = lightGrid[tileIndex].count;
    uint lightIndexOffset = lightGrid[tileIndex].offset;

    // Reading from the global light list and calculating the radiance contribution of each light.
    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 viewDir = GetViewDir(fs_in.posWS);

    // directional lights
    for (int i = 0; i < dirLightNum; i++)
      FragColor += vec4(CalcDirLight(i, normal, viewDir), 0.0);
    
    // point lights
    for (uint i = 0; i < lightCount; i++)
    {
      uint lightVectorIndex = globalLightIndexList[lightIndexOffset + i];
      //FragColor += vec4(pointLight[lightVectorIndex].color.rgb, 0.0);
      FragColor += vec4(calcPointLight(lightVectorIndex, normal, fs_in.posWS, viewDir, diffTex), 0.0);
    }
    FragColor.rgb *= diffTex.rgb;
    FragColor.rgb = max(FragColor.rgb, globalAmbient);

    // apply fog
    float S = FogAttenuation(fs_in.posWS);
    FragColor = mix(vec4(fogColor, 1.0), FragColor, S);
  }

  vec3 CalcDirLight(int index, vec3 normal, vec3 viewDir)
  {
    vec3 lightDir = normalize(-dirLights[index].direction.xyz);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = pow(max(dot(normal, halfwayDir), 0.0), property.shininess);

    // combine results
    vec3 diffuse = dirLights[index].color.rgb * diff;
    vec3 specular = dirLights[index].color.rgb * spec;
    vec3 color = (diffuse + specular);

    // calculate shadow
    vec3 shadow = DirLightShadow(index, fs_in.posWS, 0.0, normal);
    color *= shadow;
    
    vec3 LN = lightDir + normal * property.sssDistort;
    float sssDot = pow(clamp(dot(viewDir, -LN), 0.0, 1.0), property.sssPower) * property.sssScale;
    vec3 sss = property.sssAtten * (sssDot * dirLights[index].color.rgb) * 1.0;
    color += sss;

    return color;
  }
  
  float pow2(float val)
  {
    return val * val;
  }

  vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos,
                      vec3 viewDir, vec3 albedo)
  {
    //Point light basics
    vec3 position = pointLight[index].position.xyz;
    vec3 color = pointLight[index].intensity * pointLight[index].color.rgb;
    float radius = pointLight[index].range;

    //Stuff common to the BRDF subfunctions 
    vec3 lightDir = normalize(position - fragPos);
    vec3 halfway = normalize(lightDir + viewDir);
    float nDotV = max(dot(normal, viewDir), 0.0);
    float nDotL = max(dot(normal, lightDir), 0.0);

    //Attenuation calculation that is applied to all
    float distance = length(position - fragPos);
    float attenuation = pow2(clamp(1 - (distance / radius), 0.0, 1.0)) / (1.0 + (distance * distance));
    vec3 radianceIn = color * attenuation;

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), property.shininess);

    vec3 radiance = radianceIn * nDotL;// * ((property.diffuse / M_PI) + spec );
    
    //float shadow = PointLightShadow(index, fragPos);
    //radiance *= shadow;
    
    vec3 LN = -lightDir + normal * property.sssDistort;
    float sssDot = pow(clamp(dot(viewDir, LN), 0.0, 1.0), property.sssPower) * property.sssScale;
    vec3 sss = property.sssAtten * (sssDot * color * attenuation) * 1.0;
    radiance += sss;

    return radiance;
  }
}