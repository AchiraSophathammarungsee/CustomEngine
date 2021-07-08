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
{
  in vec2 UV;
  
  uniform sampler2DArray texArray;
  uniform vec2 blurScale;
  uniform vec2 nearFar;
  uniform vec2 meanVarDepth;
  
  layout(location=0) out vec3 MeanTex;
  layout(location=1) out vec3 VarianceTex;
  
  void SampleTexture(vec2 uv, float weight, inout vec3 meanSum, inout vec3 varianceSum, inout vec3 infinityWeight, inout vec3 minFiniteZ)
  {
    vec3 m;// = texture(mean, uv).rgb;
    vec3 v;// = texture(variance, uv).rgb;
    
    //vec3(greaterThan(uniformRand, absorption));
    
    if(m.r >= nearFar.y)
    {
      infinityWeight.r += weight;
    }
    else
    {
      minFiniteZ.r = max(minFiniteZ.r, m.r);
      meanSum.r += m.r * weight;
      varianceSum.r += v.r * weight;
    }
    
    if(m.g >= nearFar.y)
    {
      infinityWeight.g += weight;
    }
    else
    {
      minFiniteZ.g = max(minFiniteZ.g, m.g);
      meanSum.g += m.g * weight;
      varianceSum.g += v.g * weight;
    }
    
    if(m.b >= nearFar.y)
    {
      infinityWeight.b += weight;
    }
    else
    {
      minFiniteZ.g = max(minFiniteZ.b, m.b);
      meanSum.b += m.b * weight;
      varianceSum.b += v.b * weight;
    }
  }
  
  void GaussianLinear13x13(vec2 uv, out vec3 meanSum, out vec3 varSum)
  {
    vec2 off1 = vec2(1.4091998770852122517874022103911) * blurScale;
    vec2 off2 = vec2(3.2979348079914822508827200123103) * blurScale;
    vec2 off3 = vec2(5.2062900776825963903107721550453) * blurScale;
    
    meanSum = texture(texArray, vec3(uv, meanVarDepth.x)).rgb * 0.1976406528809569;
    varSum = texture(texArray, vec3(uv, meanVarDepth.y)).rgb * 0.1976406528809569;
    
    meanSum += texture(texArray, vec3(uv + off1, meanVarDepth.x)).rgb * 0.29598550560065581;
    meanSum += texture(texArray, vec3(uv - off1, meanVarDepth.x)).rgb * 0.29598550560065581;
    meanSum += texture(texArray, vec3(uv + off2, meanVarDepth.x)).rgb * 0.0935333619980594;
    meanSum += texture(texArray, vec3(uv - off2, meanVarDepth.x)).rgb * 0.0935333619980594;
    meanSum += texture(texArray, vec3(uv + off3, meanVarDepth.x)).rgb * 0.0116608059608062895;
    meanSum += texture(texArray, vec3(uv - off3, meanVarDepth.x)).rgb * 0.0116608059608062895;
    
    varSum += texture(texArray, vec3(uv + off1, meanVarDepth.y)).rgb * 0.29598550560065581;
    varSum += texture(texArray, vec3(uv - off1, meanVarDepth.y)).rgb * 0.29598550560065581;
    varSum += texture(texArray, vec3(uv + off2, meanVarDepth.y)).rgb * 0.0935333619980594;
    varSum += texture(texArray, vec3(uv - off2, meanVarDepth.y)).rgb * 0.0935333619980594;
    varSum += texture(texArray, vec3(uv + off3, meanVarDepth.y)).rgb * 0.0116608059608062895;
    varSum += texture(texArray, vec3(uv - off3, meanVarDepth.y)).rgb * 0.0116608059608062895;
  }
  
  void GaussianLinear17x17(vec2 uv, out vec3 meanSum, out vec3 varSum)
  {
    vec2 off1 = vec2(1.4091998770852122521533126911081) * blurScale;
    vec2 off2 = vec2(3.2979348079914822370220398004167) * blurScale;
    vec2 off3 = vec2(5.2062900776825964408709580695111) * blurScale;
    vec2 off4 = vec2(7.1372707745295434052364200571962) * blurScale;
    
    /* meanSum = texture(mean, uv).rgb * 0.1974167643837327;
    varSum = texture(variance, uv).rgb * 0.1974167643837327;
    
    meanSum += texture(mean, uv + off1).rgb * 0.29565021147425467;
    meanSum += texture(mean, uv - off1).rgb * 0.29565021147425467;
    meanSum += texture(mean, uv + off2).rgb * 0.093427406853949024;
    meanSum += texture(mean, uv - off2).rgb * 0.093427406853949024;
    meanSum += texture(mean, uv + off3).rgb * 0.011647596531041045;
    meanSum += texture(mean, uv - off3).rgb * 0.011647596531041045;
    meanSum += texture(mean, uv + off4).rgb * 0.00056640294888885768;
    meanSum += texture(mean, uv - off4).rgb * 0.00056640294888885768;
    
    varSum += texture(variance, uv + off1).rgb * 0.29565021147425467;
    varSum += texture(variance, uv - off1).rgb * 0.29565021147425467;
    varSum += texture(variance, uv + off2).rgb * 0.093427406853949024;
    varSum += texture(variance, uv - off2).rgb * 0.093427406853949024;
    varSum += texture(variance, uv + off3).rgb * 0.011647596531041045;
    varSum += texture(variance, uv - off3).rgb * 0.011647596531041045;
    varSum += texture(variance, uv + off4).rgb * 0.00056640294888885768;
    varSum += texture(variance, uv - off4).rgb * 0.00056640294888885768; */
  }

  void main()
  {
    //ivec2 uv = ivec2(gl_FragCoord.xy) << shift;
    
    /* vec3 minFiniteZ = vec3(0.0);
    vec3 infinityWeight = vec3(0.0);
    
    vec3 meanSum = vec3(0);
    vec3 varianceSum = vec3(0);
    
    SampleTexture(fs_in.UV, 0.1974167643837327, meanSum, varianceSum, infinityWeight, minFiniteZ);
    SampleTexture(fs_in.UV + off1, 0.29565021147425467, meanSum, varianceSum, infinityWeight, minFiniteZ);
    SampleTexture(fs_in.UV - off1, 0.29565021147425467, meanSum, varianceSum, infinityWeight, minFiniteZ);
    SampleTexture(fs_in.UV + off2, 0.093427406853949024, meanSum, varianceSum, infinityWeight, minFiniteZ);
    SampleTexture(fs_in.UV - off2, 0.093427406853949024, meanSum, varianceSum, infinityWeight, minFiniteZ);
    SampleTexture(fs_in.UV + off3, 0.011647596531041045, meanSum, varianceSum, infinityWeight, minFiniteZ);
    SampleTexture(fs_in.UV - off3, 0.011647596531041045, meanSum, varianceSum, infinityWeight, minFiniteZ);
    SampleTexture(fs_in.UV + off4, 0.00056640294888885768, meanSum, varianceSum, infinityWeight, minFiniteZ);
    SampleTexture(fs_in.UV - off4, 0.00056640294888885768, meanSum, varianceSum, infinityWeight, minFiniteZ);
    
    // Choose a Z value to use for "infinity" that isn't too extreme
    vec3 infinityZ = vec3(nearFar.y);
    
    if (minFiniteZ.r > 0)
    {
        // In the cas where there are some finite taps, choose the closer to zero of the finite taps and infinity.
        // The multiplier on minFiniteZ controls how opaque these edge transparent elements are for shadows.
        // Higher multiplier = more transparent.
        infinityZ.r = min(infinityZ.r, minFiniteZ.r * 1.4 - 1.0);
    }
    if (minFiniteZ.g > 0)
    {
      infinityZ.g = min(infinityZ.g, minFiniteZ.g * 1.4 - 1.0);
    }
    if (minFiniteZ.b > 0)
    {
      infinityZ.b = min(infinityZ.b, minFiniteZ.b * 1.4 - 1.0);
    }
    
    // Weigh the accumulated infinite taps
    meanSum += infinityZ * infinityWeight;
    varianceSum += infinityZ * infinityZ * infinityWeight;
    
    if (meanSum.r >= nearFar.y * -0.999)
    {
      // This pixel is exclusively at the far plane. Force it *really* 
      // far away. This is different from pixels that mix finite and infinite
      // values, where we don't want them blended together because it would 
      // be biased towards infinity.
      meanSum.r = -1e5;
      varianceSum.r = -1e10;
    }
    if (meanSum.g >= nearFar.y * -0.999)
    {
      meanSum.g = -1e5;
      varianceSum.g = -1e10;
    }
    if (meanSum.b >= nearFar.y * -0.999)
    {
      meanSum.b = -1e5;
      varianceSum.b = -1e10;
    }
    MeanTex = meanSum;
    VarianceTex = varianceSum; */
    
    vec3 meanSum;
    vec3 varSum;
    
    GaussianLinear13x13(UV, meanSum, varSum);
    
    MeanTex = meanSum;
    VarianceTex = varSum;
  }
}