vec2 computeRefractionOffset(float backgroundZ, vec2 backgroundSizeMeter, vec3 csN, vec3 csPosition, float etaRatio);

void WriteOIT(vec3 premultipliedEmission, float alpha, vec3 transmission, float collimination, float etaRatio, vec3 csPosition, vec3 csNormal, vec2 backgroundSizeMeter, sampler2D depthTex,
              out vec4 ACCUM, out vec4 MODULATE, out vec2 REFRACTION)
{
  /* Perform this operation before modifying the coverage to account for transmission */
  MODULATE.rgb = alpha * (vec3(1.0) - transmission);
    
  /* Modulate the net coverage for composition by the transmission. This does not affect the color channels of the
  transparent surface because the caller's BSDF model should have already taken into account if transmission modulates
  reflection. See:

  McGuire and Enderton, Colored Stochastic Shadow Maps, ACM I3D, February 2011
  http://graphics.cs.williams.edu/papers/CSSM/

  for a full explanation and derivation.*/
  float netCoverage = alpha * (1.0 - dot(transmission, vec3(1.0/3.0)));
  
  if (etaRatio != 0.0)
  {
    /* if (gl_FrontFacing == true)
    {
      // Backface
      etaRatio = 1.0 / etaRatio;
    } */
      
    float backgroundZ = csPosition.z - 2;
    vec2 refractionOffset = computeRefractionOffset(backgroundZ, backgroundSizeMeter, csNormal, csPosition, etaRatio);
    // Encode into snorm. Maximum offset is 1 / 8 of the screen
    REFRACTION.rg = refractionOffset * netCoverage * 8.0;

    if (etaRatio > 1.0)
    {
      // Exiting; probably the back surface. Dim reflections 
      // based on the assumption of traveling through the medium.
      premultipliedEmission *= transmission * transmission; 
    }
  }
  else
  {
    REFRACTION.rg = vec2(0);
  }
    
  float tmp = 1.0 - gl_FragCoord.z * 0.99; 
  //tmp *= netCoverage * 10.0;
  tmp *= tmp * tmp * 1e4;
  tmp = clamp(tmp, 1e-3, 1.0);
    
  /* Weight function tuned for the general case */
  float weight = clamp(netCoverage * tmp, 1e-3, 1.5e2);
  //float weight = clamp(tmp*tmp*tmp, 0.01, 30.0);
    
  // diffusion
  //const float k_0 = 120.0 / 63.0, k_1 = 0.05;
  //float z_B = linearDepth(texture(depthTex, gl_FragCoord.xy).r);
  MODULATE.a = 1.0;//k_0 * netCoverage * (1.0-collimination) * (1.0-k_1/(k_1+csPosition.z-z_B))/(1.0/63.0);
    
  ACCUM = vec4(premultipliedEmission * alpha, netCoverage) * weight;
}

/** Result is in texture coordinates */
vec2 computeRefractionOffset(float backgroundZ, vec2 backgroundSizeMeter, vec3 csN, vec3 csPosition, float etaRatio)
{
  //if (etaRatio > 1.0)
  //{
    // Exiting. As a hack, eliminate refraction on the back
    // surface to help compensate for the fixed-distance assumption
    //return vec2(0);
  //}

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
    vec2 backCoord = (hit / backgroundSizeMeter) + vec2(0.5);
      
    backCoord.y = 1.0 - backCoord.y;

    vec2 startCoord = (csPosition.xy / backgroundSizeMeter) + vec2(0.5);
  
    vec2 delta = backCoord - startCoord;
    return delta * 0.4;
  }
}