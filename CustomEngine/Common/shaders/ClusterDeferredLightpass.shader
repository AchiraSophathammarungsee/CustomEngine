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
#include ../Common/shaderlib/common.glsl
#include ../Common/shaderlib/clustered.glsl
#include ../Common/shaderlib/lighting.glsl
{
  out vec4 FragColor;
  
  uniform sampler2D fbo_tex0;
  uniform sampler2D fbo_tex1;
  uniform sampler2D fbo_tex2;
  uniform sampler2D fbo_tex3;

  in vec2 TexCoords;

  // Function prototypes
  vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, vec3 viewDir, float shininess);
  vec3 CalcDirLight(int index, vec3 worldPos, vec3 normal, vec3 viewDir, float shininess);

  void main()
  {
    vec3 WorldPos = texture(fbo_tex0, TexCoords).rgb;
    vec3 Normal = texture(fbo_tex1, TexCoords).rgb;
    vec3 Diffuse = texture(fbo_tex2, TexCoords).rgb;
    vec3 Specular = texture(fbo_tex3, TexCoords).rgb;
    float Shininess = texture(fbo_tex3, TexCoords).a;

    // Locating which cluster you are a part of
    vec4 clipSpacePos = finalMatrix * vec4(WorldPos, 1);
    vec3 ndcCoord = clipSpacePos.xyz / clipSpacePos.w;
    uint tileIndex = CurrentCluster((ndcCoord + 1.0) * 0.5 * vec3(screenParam, 1));

    // Point lights
    uint lightCount = lightGrid[tileIndex].count;
    uint lightIndexOffset = lightGrid[tileIndex].offset;

    // Reading from the global light list and calculating the radiance contribution of each light.
    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 viewDir = GetViewDir(WorldPos);

    // directional lights
    for (int i = 0; i < dirLightNum; i++)
      FragColor += vec4(CalcDirLight(i, WorldPos, Normal, viewDir, Shininess), 0.0);
    
    // point lights
    for (uint i = 0; i < lightCount; i++)
    {
      uint lightVectorIndex = globalLightIndexList[lightIndexOffset + i];
      //FragColor += vec4(pointLight[lightVectorIndex].color.rgb, 0.0);
      FragColor += vec4(calcPointLight(lightVectorIndex, Normal, WorldPos, viewDir, Shininess), 0.0);
    }
    FragColor.rgb *= Diffuse.rgb;
    FragColor.rgb = max(FragColor.rgb, globalAmbient);

    // apply fog
    float S = FogAttenuation(WorldPos);
    FragColor = mix(vec4(fogColor, 1.0), FragColor, S);
  }

  vec3 CalcDirLight(int index, vec3 worldPos, vec3 normal, vec3 viewDir, float shininess)
  {
    vec3 lightDir = normalize(-dirLights[index].direction.xyz);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    // combine results
    vec3 diffuse = dirLights[index].color.rgb * diff;
    vec3 specular = dirLights[index].color.rgb * spec;
    vec3 color = (diffuse + specular);

    // calculate shadow
    vec3 shadow = DirLightShadow(index, worldPos, 0.0, normal);
    color *= shadow;

    return color;
  }
  
  float pow2(float val)
  {
    return val * val;
  }

  vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, vec3 viewDir, float shininess)
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
    float attenuation = pow2(clamp(1 - (distance / radius), 0.0, 1.0)) / (1.0 + (distance * distance));
    vec3 radianceIn = color * attenuation;

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 radiance = radianceIn * nDotL;
    
    //float shadow = PointLightShadow(index, fragPos);
    //radiance *= shadow;

    return radiance;
  }
}