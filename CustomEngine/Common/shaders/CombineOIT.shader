Property
{
}

#VERT
{
  // Input vertex data, different for all executions of this shader.
  layout(location = 0) in vec3 vPosition;
  layout(location = 1) in vec3 vNormal;
  layout(location = 2) in vec3 vTangent;
  layout(location = 3) in vec3 vBitangent;
  layout(location = 4) in vec2 vUV;
  layout(location = 5) in vec4 vBoneIDs;
  layout(location = 6) in vec4 vWeights;

  out vec2 UV;

  void main()
  {
    UV = vUV;
    gl_Position = vec4(2.f * vPosition, 1.0f);
  }
}

#FRAG
#include ../Common/shaderlib/common.glsl
{
  out vec4 FragColor;

  in vec2 UV;
  
  uniform sampler2D ATex;
  uniform sampler2D BDTex;
  uniform sampler2D DeltaTex;
  uniform sampler2D BGTex;
  
  float minComponent(vec3 val)
  {
    return min(min(val.r, val.g), val.b);
  }
  
  float maxComponent(vec3 val)
  {
    return max(max(val.r, val.g), val.b);
  }
  
  vec3 spectralWeight(float t)
  {
    vec3 w;
    w.r = smoothstep(0.5, 1./3, t);
    w.b = smoothstep(0.5, 2./3., t);
    w.g = 1.0 - w.r - w.b;
    return w;
  }
  
  ivec2 trimBandThickness = ivec2(0);

  void main()
  {
    vec4 BD = texture(BDTex, UV).rgba;
    vec3 B = BD.rgb;
    if(minComponent(B) == 1.0)
    {
      // No transparency
      // Save the blending and color texture fetch cost
      FragColor = vec4(texture(BGTex, UV).rgb, 1.0);
      return;
    }
    
    vec2 delta = texture(DeltaTex, UV).rg * 0.375;
    vec4 A = texture(ATex, UV).rgba;
    
    // Suppress under and over flow
    if(isinf(A.a))
    {
      A.a = maxComponent(A.rgb);
    }
    if(isinf(maxComponent(A.rgb)))
    {
      A = vec4(isinf(A.a) ? 1.0 : A.a);
    }
    
    // Attempt to fake transmission on the additive term by blending in a little bit of the 
    // background modulation.
    {
        const float epsilon = 0.001;
        A.rgb *= vec3(0.5) + max(B, vec3(epsilon)) / (2.0 * max(epsilon, maxComponent(B)));
    }
    
    //bkg = textureLod(BGTex, delta + UV / bkgSize, 0).rgb;
    // Refraction
    vec3 bkg = vec3(0);
    vec2 bkgSize = textureSize(BGTex, 0).xy;
    vec2 bkgInvSize = 1.0 / bkgSize;
    {
      vec2 MIN = vec2(trimBandThickness + 0.5);
      vec2 MAX = bkgSize - vec2(trimBandThickness * 2.0 - 0.5f);

      float deltaMag = dot(delta, delta);
      if (deltaMag > 0)
      {
        // Chromatic abberation
        vec3 weightSum = vec3(0);
        bkg = vec3(0);

        // Take 2 * R + 1 samples
        const int R = 2;

        // Spread the samples by this percentage of delta
        const float scale = 0.05;
        for (int i = -R; i <= R; ++i)
        {
          vec3 weight = spectralWeight(0.5 + float(i) / float(R));
          vec2 texCoord = clamp(delta * bkgSize * (1.0 + float(i) * scale / float(R)) + UV * bkgSize, MIN, MAX) * bkgInvSize;
          bkg += weight * texture(BGTex, texCoord).rgb;
          weightSum += weight;
        }

        bkg /= weightSum;

      }
      else
      {
        //bkg = texture(BGTex, clamp(delta * bkgSize + UV, MIN, MAX) * bkgInvSize).rgb;
        bkg = texture(BGTex, UV).rgb;
      }
    }
    
    // Pixels per unit diffusion std dev
    /*
    const float PPD = 200.0;
    const int maxDiffusionPixels = 16;
    float D2 = BD.a * PPD*PPD;
    ivec2 C = ivec2(gl_FragCoord.xy);
    if(D2 > 0)
    {
      C += ivec2(delta * bkgSize);
      // Tap spacing
      const float stride = 2.0;
      // Kernel radius
      int R = int(int(min(sqrt(D2), maxDiffusionPixels) /
      float(stride)) * stride);
      float weightSum = 0;
      for (vec2 q = vec2(-R); q.x <= R; q.x+=stride)
      {
        for (q.y = -R; q.y <= R; q.y+=stride)
        {
          float radius2 = dot(q, q);
          if (radius2 <= D2)
          {
            ivec2 tap = C + ivec2(q);
            float t = texture(BDTex, vec2(tap / bkgSize)).a;
            float bkgRadius2 = t * PPD * PPD;
            if (radius2 <= bkgRadius2)
            {
              // Disk filter (faster, looks similar)
              float w = 1.0 / bkgRadius2 + 1e-5;
              // True Gaussian filter
              //float w=exp(-radius2 / (8*bkgRadius2)) /
              // sqrt(4 * PI * t);
              bkg += w * texture(BGTex, vec2(tap / bkgSize)).rgb;
              weightSum += w;
      }}}}
      bkg /= weightSum;
    }
    else
    {
      bkg = texture(BGTex, delta + UV).rgb;
    }*/
    
    // dst' =  (accum.rgb / accum.a) * (1 - backgroundModulation) + dst
    // [dst has already been modulated by the transmission colors and coverage and the blend mode
    // inverts revealage for us] 
    //FragColor.rgb = bkg * B + (vec3(1) - B) * A.rgb / max(A.a, 0.00001);
    FragColor.rgb = bkg * B + A.rgb / max(A.a, 0.00001);
    FragColor.a = 1.0;
  }
}