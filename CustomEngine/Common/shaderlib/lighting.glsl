struct DirLight
{
  vec4 direction;
  vec4 color;

  mat4 lightSpaceMatrix;
  int castShadow;
  float bias;
};
#define DIR_LIGHTS_NUM 16

layout(std140, binding = 7) uniform dirLightsUBO
{
  int dirLightNum;                     // 4
  int shadowMapTilesPerRow;            // 4
  DirLight dirLights[DIR_LIGHTS_NUM];  // 100 * DIR_LIGHTS_NUM
};

#define MAX_CASCADE 4

uniform int cascadeCount;
uniform int cascadeVSMCount;
uniform float cascadeEdgeBlend;
uniform float cascadeDepths[MAX_CASCADE];
uniform mat4 cascadeMatrices[MAX_CASCADE];
uniform sampler2DArray shadowOpaque;
uniform sampler2DArray shadowTransparent;
uniform samplerCube pointShadow;
uniform float VSMMin;
uniform float VSMBleedReduct;
uniform vec2 MSMBleedReductDarken;

float abssum(vec2 v) 
{
  return abs(v.x) + abs(v.y);
}

float linstep(float low, float high, float v)
{
	return clamp((v-low)/(high-low), 0.0, 1.0);
}

float sampleVarianceShadow(sampler2D shadowMap, vec2 coords, float compare)
{
  vec2 moments = texture(shadowMap, coords).xy;
  float p = step(compare, moments.x);
  float variance = max(moments.y - moments.x * moments.x, 0.00002);
  float d = compare - moments.x;
  float pMax = linstep(0.2, 1.0, variance / (variance + d*d));
  
  return min(max(p, pMax), 1.0);
}

float sampleVarianceColorShadow(float color, float alpha, float compare)
{
  vec2 moments = vec2(color, alpha);
  float p = step(compare, moments.x);
  float variance = max(moments.y - moments.x * moments.x, VSMMin);
  float d = compare - moments.x;
  float pMax = linstep(VSMBleedReduct, 1.0, variance / (variance + d*d));
  
  return min(max(p, pMax), 1.0);
}

float sampleVarianceShadowCube(samplerCube shadowMap, vec3 coords, float compare)
{
  vec2 moments = texture(shadowMap, coords).xy;
  float p = step(compare, moments.x);
  float variance = max(moments.y - moments.x * moments.x, 0.00002);
  float d = compare - moments.x;
  // linstep prevent shadow bias
  float pMax = linstep(0.2, 1.0, variance / (variance + d*d));
  
  return min(max(p, pMax), 1.0);
}

vec3 ColorVarianceShadow(float meanLayer, float varianceLayer, vec3 projCoords)
{
  vec3 mean = texture(shadowTransparent, vec3(projCoords.xy, meanLayer)).rgb;
  vec3 variance = texture(shadowTransparent, vec3(projCoords.xy, varianceLayer)).rgb;
  vec3 shadow;
  shadow.r = sampleVarianceColorShadow(mean.r, variance.r, projCoords.z);
  shadow.g = sampleVarianceColorShadow(mean.g, variance.g, projCoords.z);
  shadow.b = sampleVarianceColorShadow(mean.b, variance.b, projCoords.z);
  return shadow;
}

vec3 ColorPCFShadow(sampler2D shadowMapTex, vec3 projCoords, vec3 normal, vec3 lightDir, float lightBias)
{
  float bias = max(lightBias * (1.0 - dot(normal, lightDir)), 0.005);
  float currentDepth = projCoords.z;
  float biasDepth = currentDepth - bias;

  // PCF
  ivec3 shadow = ivec3(0.0);
  vec2 texSize = textureSize(shadowMapTex, 0);
  vec2 texelSize = 1.0 / texSize;// * tileSize;
  
  //float rand = uniformRand(projCoords.xyz);
  //float rand2 = uniformRand(projCoords.yxz);
  //vec2 randCoord = projCoords.xy * texSize;
  ivec2 c = ivec2(0,0);//ivec2(randCoord.xy + 
            //(mod(randCoord.xy * 5.0, vec2(2.0)) - vec2(1.0))  * (0.15 / vec2(abssum(dFdx(randCoord.xy)), abssum(dFdy(randCoord.xy)))) ); 
  
  vec3 shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(-9, 0) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));
  //return shadow;
  
  shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(0, -9) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));
  shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(0, 9) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));
  shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(9, 0) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));
  
  shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(4, 0) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));
  shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(-4, 0) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));
  shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(0, 4) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));
  shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(0, -4) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));
  
  shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(0, 0) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));
  
  shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(-3, -3) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));
  shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(-3, 3) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));
  shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(3, 3) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));
  shadowMap = texture(shadowMapTex, projCoords.xy + (vec2(3, -3) + c) * texelSize).rgb;
  shadow += ivec3(greaterThan(shadowMap, vec3(biasDepth)));

  return shadow / 13.0;
}

vec4 ConvertMoment(vec4 moments)
{
  const mat4 matr = mat4(0.2227744146, 0.1549679261, 0.1451988946, 0.163127443,
                         0.0771972861, 0.1394629426, 0.2120202157, 0.2591432266,
                         0.7926986636,0.7963415838, 0.7258694464, 0.6539092497,
                         0.0319417555,-0.1722823173,-0.2758014811,-0.3376131734);
                         
  moments.r -= 0.0359558848;
  
  return matr * moments;
}

float FMA(float a,float b,float c)
{
  return fma(a,b,c);
}

float MomentShadow(vec3 projCoords, float texLayer, float momentBias)
{
  vec4 moments = texture(shadowOpaque, vec3(projCoords.xy, texLayer)).rgba;
  float fragDepth = projCoords.z;
  
  // inverse moments back
  moments = ConvertMoment(moments);
  vec3 z;
  z.x = fragDepth;
  
  // Compute a Cholesky factorization of the Hankel matrix B storing only non-trivial entries or related products
  float L32D22 = FMA(-moments.x, moments.y, moments.z);
  float D22 = FMA(-moments.x, moments.x, moments.y);
  float squaredDepthVariance = FMA(-moments.y, moments.y, moments.a);
  float D33D22 = dot(vec2(squaredDepthVariance, -L32D22), vec2(D22, L32D22));
  float InvD22 = 1.0 / D22;
  float L32 = L32D22 * InvD22;
  
  // Obtain a scaled inverse image of bz = (1,z[0],z[0]*z[0])^T
  vec3 c = vec3(1.0, z.x, z.x * z.x);
  
  // Forward substitution to solve L*c1=bz
  c[1] -= moments.x;
  c[2] -= moments.y + L32 * c[1];
  
  // Scaling to solve D*c2=c1
  c[1] *= InvD22;
  c[2] *= D22 / D33D22;
  
  // Backward substitution to solve L^T*c3=c2
  c[1] -= L32 * c[2];
  c[0] -= dot(c.yz, moments.xy);
  
  // Solve the quadratic equation c[0]+c[1]*z+c[2]*z^2 to obtain solutions z[1] and z[2]
  float p = c[1] / c[2];
  float q = c[0] / c[2];
  float D = (p * p * 0.25) - q;
  float r = sqrt(D);
  z[1] =- p * 0.5 - r;
  z[2] =- p * 0.5 + r;
  
  // Compute the shadow intensity by summing the appropriate weights
  // Using MSM Hausdorff
  float shadowIntensity = 1.0;
  
  // Use a solution made of four deltas if the solution with three deltas is invalid
  if(z[1] < 0.0 || z[2] > 1.0)
  {
    float zFree = ((moments[2] - moments[1]) * z[0] + moments[2] - moments[3]) / ((moments[1] - moments[0]) * z[0] + moments[1] - moments[2]);
    float w1Factor = (z[0] > zFree) ? 1.0 : 0.0;
    shadowIntensity = (moments[1] - moments[0] + (moments[2] - moments[0] - (zFree + 1.0) * (moments[1] - moments[0])) * (zFree - w1Factor - z[0]) / 
                      (z[0] * (z[0] - zFree))) / (zFree - w1Factor) + 1.0 - moments[0];
  }
  // Use the solution with three deltas
  else
  {
    vec4 switchVal = (z[2] < z[0]) ? vec4(z[1], z[0], 1.0, 1.0) :
                    ((z[1] < z[0]) ? vec4(z[0], z[1], 0.0, 1.0) :
                    vec4(0.0, 0.0, 0.0, 0.0));
    float quotient = (switchVal[0] * z[2] - moments[0] * (switchVal[0] + z[2]) + moments[1]) / ((z[2] - switchVal[1]) * (z[0] - z[1]));
    shadowIntensity = switchVal[2] + switchVal[3] * quotient;
  }
  shadowIntensity = 1 - clamp(shadowIntensity, 0, 1);
  
  // use smoothstep for light bleeding reduction
  shadowIntensity = smoothstep(MSMBleedReductDarken.x, 1.0, shadowIntensity);
  shadowIntensity = MSMBleedReductDarken.y + (1.0 - MSMBleedReductDarken.y) * shadowIntensity;
  return shadowIntensity;
}

float PCFShadow(vec3 projCoords, float texLayer, float bias, vec2 invSize)
{
  float currentDepth = projCoords.z;
  float biasDepth = currentDepth - bias;
  
  // "side" and "diagonal" offset variables used to produce vectors to the 
  // 8-neighbors, which leads to a smoother shadow result (0.71 is sqrt(2)/2).
  vec4 s = vec4(invSize, -invSize.x, 0.0);
  vec4 d = s * 1.4;// * 0.71;
  
  float opaqueShadow = 0.0;//texture(shadowOpaque, vec3(projCoords.xy, texLayer)).r < biasDepth ? 0.0 : 1.0;
  
  vec2 halfPixel = 1 * invSize;
  vec4 cmpDepth = vec4(biasDepth);
  vec4 shadowGather = textureGather(shadowOpaque, vec3(projCoords.xy + halfPixel, texLayer));
  ivec4 result = ivec4(greaterThan(shadowGather, cmpDepth));
  opaqueShadow += result.x + result.y + result.z + result.w;
  
  shadowGather = textureGather(shadowOpaque, vec3(projCoords.xy - halfPixel, texLayer));
  result = ivec4(greaterThan(shadowGather, cmpDepth));
  opaqueShadow += result.x + result.y + result.z + result.w;
  
  shadowGather = textureGather(shadowOpaque, vec3(projCoords.xy + vec2(halfPixel.x, -halfPixel.y), texLayer));
  result = ivec4(greaterThan(shadowGather, cmpDepth));
  opaqueShadow += result.x + result.y + result.z + result.w;
  
  shadowGather = textureGather(shadowOpaque, vec3(projCoords.xy + vec2(-halfPixel.x, halfPixel.y), texLayer));
  result = ivec4(greaterThan(shadowGather, cmpDepth));
  opaqueShadow += result.x + result.y + result.z + result.w;
  
  return opaqueShadow / 16.0;
  
  /* opaqueShadow += texture(shadowOpaque, vec3(projCoords.xy + s.xw, texLayer)).r < biasDepth ? 0.0 : 1.0;
  opaqueShadow += texture(shadowOpaque, vec3(projCoords.xy - s.xw, texLayer)).r < biasDepth ? 0.0 : 1.0;
  opaqueShadow += texture(shadowOpaque, vec3(projCoords.xy + s.wy, texLayer)).r < biasDepth ? 0.0 : 1.0;
  opaqueShadow += texture(shadowOpaque, vec3(projCoords.xy - s.wy, texLayer)).r < biasDepth ? 0.0 : 1.0;
  
  opaqueShadow += texture(shadowOpaque, vec3(projCoords.xy + d.xy, texLayer)).r < biasDepth ? 0.0 : 1.0;
  opaqueShadow += texture(shadowOpaque, vec3(projCoords.xy - d.zy, texLayer)).r < biasDepth ? 0.0 : 1.0;
  opaqueShadow += texture(shadowOpaque, vec3(projCoords.xy + d.zy, texLayer)).r < biasDepth ? 0.0 : 1.0;
  opaqueShadow += texture(shadowOpaque, vec3(projCoords.xy - d.xy, texLayer)).r < biasDepth ? 0.0 : 1.0;
  
  return opaqueShadow * 0.11111111111111111111111111111111; */
}

const vec3 cascadeCol[MAX_CASCADE] = vec3[] (vec3(1,0,0), vec3(0,1,0), vec3(0,0,1), vec3(1,0,1));

vec3 DirLightShadow(int index, vec3 worldPos, float clipPosZ, vec3 normal)
{
  // return if no shadow or surface facing opposite direction (doesn't receive light)
  if ((dirLights[index].castShadow == 0))
  {
    return vec3(1.0);
  }
  
  float cascadeIndex = 0.0;
  float edgeBlendRate = 0.0;
  /*for (int i = 0 ; i < cascadeCount ; i++)
  {
    float edgeZ = cascadeDepths[i] * nearFar.y;
    if (clipPosZ <= edgeZ)
    {
      float cascadeNear = cascadeDepths[max(i - 1, 0)] * nearFar.y;
      float diff = clipPosZ - cascadeNear;
      edgeBlendRate = max(0.0, cascadeEdgeBlend * nearFar.y - diff) / nearFar.y;
      
      cascadeIndex = i;
      break;
    }
  }*/
  int cascadeID = 0;//int(floor(cascadeIndex));
  //return cascadeCol[int(floor(cascadeIndex))];
  
  //vec4 fragPosLightSpace = dirLights[index].lightSpaceMatrix * vec4(worldPos, 1.0);
  vec4 fragPosLightSpace = cascadeMatrices[cascadeID] * vec4(worldPos, 1.0);
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

  projCoords = projCoords * 0.5 + 0.5;
  
  //projCoords.z = clamp(projCoords.z, 0.0, 1.0);
  //if (projCoords.z > 1.0) return vec3(1.0, 0.0, 0.0);
  
  float bias = max(dirLights[index].bias * (1.0 - dot(normal, dirLights[index].direction.xyz)), 0.005);
  vec2 invSize = 1.f / textureSize(shadowOpaque, 0).xy;
  
  float texLayer = GetTextureArrayDepth(cascadeIndex, cascadeCount);
  //float opaqueShadow = MomentShadow(projCoords, texLayer, dirLights[index].bias);//PCFShadow(projCoords, texLayer, bias, invSize);
  float opaqueShadow = PCFShadow(projCoords, texLayer, bias, invSize);
  
  /*if((edgeBlendRate != 0.0) && (cascadeID < cascadeCount))
  {
    vec4 secondLightSpace = cascadeMatrices[cascadeID + 1] * vec4(worldPos, 1.0);
    vec3 secondprojCoords = secondLightSpace.xyz / secondLightSpace.w;
    secondprojCoords = secondprojCoords * 0.5 + 0.5;
    float sectexLayer = max(0, min(float(cascadeCount) - 1, floor(cascadeIndex + 1 + 0.5)));
    opaqueShadow = mix(PCFShadow(secondprojCoords, sectexLayer, bias, invSize), opaqueShadow, edgeBlendRate);
  } */
  
  vec3 transparentShadow = vec3(1.0);
  if(cascadeID < cascadeVSMCount)
  {
    float vsmMeanLayer = cascadeIndex * 2;
    float vsmVarianceLayer = cascadeIndex * 2 + 1;
    int totalLayers = 2 * cascadeVSMCount + 2; // *2 for mean and variance, +2 for blur tmp
    
    float meanLayer = GetTextureArrayDepth(vsmMeanLayer, totalLayers); 
    float varianceLayer = GetTextureArrayDepth(vsmVarianceLayer, totalLayers);
    transparentShadow = ColorVarianceShadow(meanLayer, varianceLayer, projCoords);
  }
  
  return opaqueShadow * transparentShadow;
  
  //vec3 shadowMap = texture(shadowMaps, projCoords.xy).rgb;
  //return vec3(greaterThan(shadowMap, vec3(biasDepth)));
  //colorShwCoords = projCoords * vec3(tileSize,tileSize,1.0) + transOffset;
  //projCoords = projCoords * vec3(tileSize,tileSize,1.0) + opaqueOffset;
}

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
); 

float PointLightShadow(uint index, vec3 fragPos)
{
  if(pointLight[index].haveShadow == 0) return 1.0;
  
  vec3 fragToLight = fragPos - pointLight[index].position.xyz;
  float currentDepth = length(fragToLight);
  currentDepth /= pointLight[index].range;
  
  float shadow = sampleVarianceShadowCube(pointShadow, fragToLight, currentDepth);
  
  //float bias = 0.05;
  //vec2 texelSize = 1.0 / textureSize(pointShadow, 0);
  //float viewDistance = length(camPos - fragPos);
  //float diskRadius = texelSize.x * (1.0 + (viewDistance / pointLight[index].range));
  
  //float shadow = 0.0;
  //for(uint i = 0; i < 20; i++)
  //{
  //  float closestDepth = texture(pointShadow, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
  //  closestDepth *= pointLight[index].range;
  //  shadow += currentDepth -  bias > closestDepth ? 0.0 : 1.0;
  //}
  
  //float closestDepth = texture(pointShadow, fragToLight).r;
  //closestDepth *= pointLight[index].range;
  //float bias = 0.05;
  //shadow = currentDepth -  bias > closestDepth ? 0.0 : 1.0;
  //shadow /= 20.0;

  return shadow;
}

