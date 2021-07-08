Property
{
  float alpha = (0.5)
  color3 ambient = (0.1,0.1,0.1)
  color3 diffuse = (0.8, 0.8, 0.8)
  color3 specular = (1, 1, 1)
  float shininess = (32.0)
  bool haveDiffMap = (false)
  bool haveNormMap = (false)
  bool haveSpecMap = (false)
  sampler2D diffMap = (1)
  sampler2D normMap = (2)
  sampler2D specMap = (3)
  float etaRatio = (1.0)
  float roughness = (1.0)
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
#include ../Common/shaderlib/common.glsl
#include ../Common/shaderlib/clustered.glsl
#include ../Common/shaderlib/lighting.glsl
#include ../Common/shaderlib/reflection.glsl
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
  
  uniform samplerCube skybox;

  // Function prototypes
  vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, vec3 viewDir);
  vec3 CalcDirLight(int index, vec3 normal, vec3 viewDir);
  
  /** Result is in texture coordinates */
  vec2 computeRefractionOffset(float backgroundZ, vec2 bgSize, vec3 csN, vec3 csPosition, float etaRatio)
  {
    if (etaRatio > 1.0)
    {
      // Exiting. As a hack, eliminate refraction on the back
      // surface to help compensate for the fixed-distance assumption
      return vec2(0);
    }

    /* Incoming ray direction from eye, pointing away from csPosition */
    vec3 csw_i = normalize(-csPosition);

    /* Refracted ray direction, pointing away from wsPos */
    vec3 csw_o = refract(-csw_i, csN, etaRatio);

    bool totalInternalRefraction = (dot(csw_o, csw_o) < 0.01);
    if (totalInternalRefraction)
    {
      /* No transmitted light */
      return vec2(0.0);
    }
    else
    {
      /* Take to the reference frame of the background (i.e., offset) */
      vec3 d = csw_o;

      /* Take to the reference frame of the background, where it is the plane z = 0 */
      vec3 P = csPosition;
      P.z -= backgroundZ;

      /* Find the xy intersection with the plane z = 0 */
      vec2 hit = (P.xy - d.xy * P.z / d.z);

      /* Hit is now scaled in meters from the center of the screen; adjust scale and offset based
        on the actual size of the background */
      vec2 backCoord = (hit / bgSize) + vec2(0.5);
      
      backCoord.y = 1.0 - backCoord.y;

      vec2 startCoord = (csPosition.xy / bgSize) + vec2(0.5);
    
      vec2 delta = backCoord - startCoord;
      return delta * 0.15;
    }
  }
  
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

    vec4 diffTex = vec4(property.diffuse.rgb, 1);
    if (property.haveDiffMap)
      diffTex *= texture(property.diffMap, fs_in.UV).rgba;

    vec3 spec = property.specular * texture(property.specMap, fs_in.UV).r;
    property.shininess;

    property.ambient;

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
      lightCol += CalcDirLight(i, normal, viewDir);

    // point lights
    for (uint i = 0; i < lightCount; i++)
    {
      uint lightVectorIndex = globalLightIndexList[lightIndexOffset + i];
      //FragColor += vec4(pointLight[lightVectorIndex].color.rgb, 0.0);
      lightCol += calcPointLight(lightVectorIndex, normal, fs_in.posWS, viewDir);
    }
    // lighting only affects surface with partial coverage
    //lightCol *= alpha;
    //lightCol = min(lightCol, 1);
    
    color.rgb = property.diffuse.rgb;
    color.rgb = max(color.rgb, globalAmbient);
    
    // tonemapping and gammacorrection
    //lightCol.rgb = lightCol.rgb / (lightCol.rgb + vec3(1.0));
    //lightCol.rgb = pow(lightCol.rgb, vec3(1.0/2.2));

    // apply fog
    float S = FogAttenuation(fs_in.posWS);
    color = mix(vec4(fogColor, 1.0), color, S);
    color.a = property.alpha * diffTex.a;
    
    // reflection
    vec3 reflection = (1.0 - property.roughness) * ReflectCubemap(skybox, fs_in.posWS, normal, camPos);
    
    // OIT WEIGHT BLEND
    
    float alpha = color.a;
    vec3 transmission = color.rgb;
    vec3 emission = clamp(lightCol + reflection, 0, 1) * alpha;
    
    /* Perform this operation before modifying the coverage to account for transmission */
    BetaOUT.rgb = alpha * (vec3(1.0) - transmission);
    
    float netCoverage = alpha * (1.0 - ((transmission.r + transmission.g + transmission.b) * (1.0/3.0)));
    
    float tmp = 1.0 - gl_FragCoord.z * 0.99; 
    tmp *= tmp * tmp * 1e4;
    tmp = clamp(tmp, 1e-3, 1.0);
    
    /* Weight function tuned for the general case */
    float weight = clamp(netCoverage * tmp, 1e-3, 1.5e2);
    
    // diffusion
    BetaOUT.a = 1.0;
    
    if (property.etaRatio != 1.0)
    {
      vec2 bgSize = vec2(32.0);
      float backgroundZ = fs_in.posCS.z - 4;
      vec2 refractionOffset = computeRefractionOffset(backgroundZ, bgSize, normalCS, fs_in.posCS.xyz, property.etaRatio);
      // Encode into snorm. Maximum offset is 1 / 8 of the screen
      RefractOUT.rg = refractionOffset * netCoverage * 8.0;

      if (property.etaRatio > 1.0)
      {
        // Exiting; probably the back surface. Dim reflections 
        // based on the assumption of traveling through the medium.
        emission *= transmission * transmission; 
      }
    }
    else
    {
      RefractOUT.rg = vec2(0);
    }
    
    AccumOUT = vec4(emission * alpha, netCoverage) * weight;
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

    // calculate shadow
    vec3 shadow = DirLightShadow(index, fs_in.posWS, 0.0, normal);

    return shadow * (diffuse + specular);
  }

  vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, vec3 viewDir)
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
    float attenuation = pow(clamp(1 - pow((distance / radius), 4.0), 0.0, 1.0), 2.0) / (1.0 + (distance * distance));
    vec3 radianceIn = color * attenuation;

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), property.shininess);

    vec3 radiance = radianceIn * nDotL;// * ((property.diffuse.rgb / M_PI) + spec );
    
    //float shadow = PointLightShadow(index, fragPos);
    //radiance *= shadow;
    
    //vec3 LN = -lightDir + normal * 1;
    //float sssDot = pow(clamp(dot(viewDir, LN), 0.0, 1.0), 1) * 1;
    //vec3 sss = 1 * (sssDot * color * attenuation) * 1.0;
    //radiance += sss;

    return radiance;
  }
}