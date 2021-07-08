Property
{
  float alpha = (0.5)
  float transmission = (1.0)
  color3 diffuse = (0.8, 0.2, 0.2)
  float etaRatio = (0.999)
  bool haveDiffMap = (false)
  sampler2D diffMap = (1)
}
#VERT
#include ../Common/shaderlib/common.glsl
#include ../Common/shaderlib/bones.glsl
{
  // Input vertex data, different for all executions of this shader.
  layout(location = 0) in vec3  vPosition;
  layout(location = 1) in vec3 vNormal;
  layout(location = 2) in vec3 vTangent;
  layout(location = 3) in vec3 vBitangent;
  layout(location = 4) in vec2 vUV;
  layout(location = 5) in vec4 vBoneIDs;
  layout(location = 6) in vec4 vWeights;
  
  uniform mat4 lightSpaceMatrix;
  
  out VS_OUT
  {
    vec4 worldPos;
    vec4 normalWS;
    vec2 UV;
  } vs_out;

  void main()
  {
    vs_out.worldPos = modelMatrix * vec4(vPosition, 1.0f );
    vs_out.normalWS = normalize(modelMatrix * vec4(vNormal, 0.0f ));
    vs_out.UV = vUV;
    gl_Position = lightSpaceMatrix * vs_out.worldPos;
  }
}
#FRAG
#include ../Common/shaderlib/mathutil.glsl
{
  layout(location=0) out vec3 shadow;
  layout(location=1) out vec3 variance;
  
  in VS_OUT
  {
    vec4 worldPos;
    vec4 normalWS;
    vec2 UV;
  } fs_in;
  
  uniform sampler2D uniformNoise;
  uniform mat4 lightSpaceMatrix;
  
  vec3 random() 
  {
    return texelFetch(uniformNoise, (ivec2(gl_FragCoord.xy) ^ ivec2(gl_FragCoord.z * 2048.0)) & 255, 0).rrr;
  }
  
  float rand(vec3 p)
  {
    return fract(sin(p.x * 1e2 + p.y) * 1e5 + sin(p.y * 1e3 + p.z * 1e2) * 1e3 + sin(p.z * 1e4 + p.x) * 1e4);
  }
  
  float randomP(vec3 p)
  {
    return fract(sin(p.x * 1e2 + p.y) * 1e5 + sin(p.y * 1e3 + p.z * 1e2) * 1e3);
  }
  
  void main()
  {
    vec3 transmission = property.diffuse * property.transmission;
    float alpha = property.alpha;
    
    if(property.haveDiffMap)
    {
      vec4 diffTex = texture(property.diffMap, fs_in.UV);
      alpha *= diffTex.a;
    }
    
    if(alpha == 0.0) discard;
    
    // Reduce everything to "coverage"
    vec3 absorption = (1.0 - transmission) * alpha;
    
    if (property.etaRatio < 1.0)
    {
      // Create approximate caustics by decreasing shadowing based on refraction and angle.
      // The "eye" vector below is the light vector when we're rendering a shadow map.
      vec3 wsEye = normalize(lightSpaceMatrix[3].xyz - fs_in.worldPos.xyz);

      // Amount of the caustic effect. amount = 0 is none (eta = 1), amount = 1 is strong.
      float amount = ((1.0 / property.etaRatio - 1.0) * (1.0 / 2.0));
      amount *= amount;

      // When the normal points into the light, we want much less absorption.
      // Let incidence = 0 for glancing angles and incidence = 1 for normal incidence.
      float incidence = abs(dot(fs_in.normalWS.xyz, vec3(0.5, -1.0, 0.0))); 

      // Where incidence is low, make the shadow *darker* (darken > 1). Where incidence 
      // is high, make the shadow less dark (darken < 1).
      // The darkness of the caustic is the opposite of the incidence, with 
      // a lot of contrast.
                
      // On the range [0, 2.0]
      float darken = clamp(1.0 - pow(incidence, 128.0 * amount), 0, 1) * 2.0;
                
      // Increase the effect with index of refraction
      darken = mix(1.0, darken, pow(amount, 0.1));

      // As etaRatio increases, we want absorption to become more like k
      // At etaRatio = 1, absorption is unmodified. At etaRatio = 3,
      // there should be extreme sensitivity to k, increasing both in contrast
      // and in rate.
      absorption *= darken;

      // Don't let coverage ever hit 1.0 due to total internal refraction
      // because that will trigger DISCARD_IF_NO_TRANSPARENCY to drop it 
      // from SVSM generation.
      absorption = min(absorption, 0.999);
    }
    
    absorption = clamp(absorption, 0, 1) * 2;
    
    // Stochastic translucency
    vec3 uniformRand = vec3(randomP(fs_in.worldPos.xyz));
    //uniformRand.r = randomP(fs_in.worldPos.xyz);
    //uniformRand.g = randomP(fs_in.worldPos.zyx);
    //uniformRand.b = randomP(fs_in.worldPos.yxz);
    //vec3 uniformRand = random();
    //vec3 uniformRand = vec3(rand(fs_in.worldPos.xyz));

    // Wherever R >  rho, leave current value (min against 1.0 = Far plane)
    // Wherever R <= rho, min against current depth
    
    // z is negative and z^2 is positive, but the z value will end up being squared before
    // it is applied anyway.
    shadow.rgb = max(vec3(greaterThan(uniformRand, absorption)), gl_FragCoord.z);
    
    // prevent shadow ance
    float dx = dFdx(shadow.r);
    float dy = dFdy(shadow.r);
    variance.r = shadow.r * shadow.r + 0.25 * (dx * dx + dy * dy);
    dx = dFdx(shadow.g);
    dy = dFdy(shadow.g);
    variance.g = shadow.g * shadow.g + 0.25 * (dx * dx + dy * dy);
    dx = dFdx(shadow.b);
    dy = dFdy(shadow.b);
    variance.b = shadow.b * shadow.b + 0.25 * (dx * dx + dy * dy);
  }
}