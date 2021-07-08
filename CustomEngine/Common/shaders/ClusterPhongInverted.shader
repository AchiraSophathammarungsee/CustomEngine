Property
{
  color3 ambient = (0.1,0.1,0.1)
  color4 diffuse = (0.8, 0.8, 0.8, 0.5)
  color3 specular = (1, 1, 1)
  float shininess = (32.0)
  bool haveDiffMap = (false)
  bool haveNormMap = (false)
  bool haveSpecMap = (false)
  sampler2D diffMap = (1)
  sampler2D normMap = (2)
  sampler2D specMap = (3)
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

    vec3 T = vec3(modelMatrix * vec4(vTangent, 0.f));
    vec3 B = vec3(modelMatrix * vec4(vBitangent, 0.f));
    vec3 N = vec3(modelMatrix * vec4(vNormal, 0.f));
    vs_out.TBN = mat3(T, B, N);

    gl_Position = finalMatrix * worldPos;
  }
}

#FRAG
#include ../Common/shaderlib/common.glsl
#include ../Common/shaderlib/lighting.glsl
#include ../Common/shaderlib/clustered.glsl
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
    //if(property.diffuse.a < 0.5) discard;

    vec3 normal;
    if (property.haveNormMap)
    {
      normal = texture(property.normMap, fs_in.UV).rgb;
      normal = normalize(normal * 2.0 - 1.0);
      normal = normalize(fs_in.TBN * normal);
    }
    else
    {
      normal = normalize(fs_in.TBN[2]);
    }

    vec3 diffTex = property.diffuse.rgb;
    if (property.haveDiffMap)
      diffTex *= texture(property.diffMap, fs_in.UV).rgb;

    vec3 spec = property.specular * texture(property.specMap, fs_in.UV).r;
    property.shininess;

    property.ambient;

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

    // apply fog
    float S = FogAttenuation(fs_in.posWS);
    FragColor = mix(vec4(fogColor, 1.0), FragColor, S);
    FragColor.a = property.diffuse.a;
    FragColor.rgb = vec3(1) - FragColor.rgb;
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
    vec3 diffuse = dirLights[index].color.rgb * diff * property.diffuse.rgb;
    vec3 specular = dirLights[index].color.rgb * spec;

    // calculate shadow
    vec3 shadow = DirLightShadow(index, fs_in.posWS, normal);

    return shadow * (diffuse + specular);
  }

  vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos,
                      vec3 viewDir, vec3 albedo)
  {
    //Point light basics
    vec3 position = pointLight[index].position.xyz;
    vec3 color = 100.0 * pointLight[index].color.rgb;
    float radius = pointLight[index].range;

    //Stuff common to the BRDF subfunctions 
    vec3 lightDir = normalize(position - fragPos);
    vec3 halfway = normalize(lightDir + viewDir);
    float nDotV = max(dot(normal, viewDir), 0.0);
    float nDotL = max(dot(normal, lightDir), 0.0);

    //Attenuation calculation that is applied to all
    float distance = length(position - fragPos);
    float attenuation = pow(clamp(1 - pow((distance / radius), 4.0), 0.0, 1.0), 2.0) / (1.0 + (distance * distance));
    vec3 radianceIn = color * attenuation;

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), property.shininess);

    vec3 radiance = radianceIn * nDotL;// * ((property.diffuse.rgb / M_PI) + spec );

    return radiance;
  }
}