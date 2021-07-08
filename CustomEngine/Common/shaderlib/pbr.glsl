// doesn't work on mobile (lack of bit op)
// use VanDerCorpus for mobile version (less performant)
float RadicalInverse_VdC(uint bits) 
{
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
  return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
  
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
  float a = roughness*roughness;

  float phi = 2.0 * M_PI * Xi.x;
  float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
  float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

  // from spherical coordinates to cartesian coordinates
  vec3 H;
  H.x = cos(phi) * sinTheta;
  H.y = sin(phi) * sinTheta;
  H.z = cosTheta;

  // from tangent-space vector to world-space sample vector
  vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangent   = normalize(cross(up, N));
  vec3 bitangent = cross(N, tangent);

  vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
  return normalize(sampleVec);
}

// statistically approximates normal distribution for surface's roughness
float DistributionGGX(vec3 norm, vec3 halfway, float roughness)
{
  float a = roughness*roughness;
  float a2 = a*a;
  float NdotH = max(dot(norm, halfway), 0.0);
  float NdotH2 = NdotH*NdotH;
  
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = M_PI * denom * denom;
  
  return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
  float denom = NdotV * (1.0 - k) + k;
  return NdotV / denom;
}

// The geometry function statistically approximates the relative surface area 
// where its micro surface-details overshadow each other, causing light rays to be occluded. 
// direct light: k = (rough + 1)^2 / 8
// IBL light: k = rough^2 / 2
float GeometrySmith(float NdotV, float NdotL, float k)
{
  float r = k + 1;
  k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
  
  float ggx1 = GeometrySchlickGGX(NdotV, k);
  float ggx2 = GeometrySchlickGGX(NdotL, k);

  return ggx1 * ggx2;
}

// The Fresnel equation (pronounced as Freh-nel) describes the ratio of light 
// that gets reflected over the light that gets refracted, which varies over the angle we're looking at a surface. 
// The moment light hits a surface, based on the surface-to-view angle, 
// the Fresnel equation tells us the percentage of light that gets reflected. 
// From this ratio of reflection and the energy conservation principle we can directly obtain the refracted portion of light. 
// cosTheta = NdotV or NdotH
// F0       = mix(F0, albedo, metallic);
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
  cosTheta = min(cosTheta, 1.0); // fix black pixels artifacts
  return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

// This will only return irridiance fresnel and exclude specular
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 SphericalHarmonicsIrradiance(vec3 sh[9], vec3 normal)
{
  vec3 result = vec3(0);
  
  //const float coeff[9] = float[9](0.282095, -0.488603, 0.488603, -0.488603, 1.092548, -1.092548, 0.315392, -1.092548, 0.546274);
  const float coeff[9] = float[9](0.282095 * 3.141593, 
                                  -0.488603 * 2.094395, 0.488603 * 2.094395, -0.488603 * 2.094395,
                                  1.092548 * 0.785398, -1.092548 * 0.785398, 0.315392 * 0.785398, -1.092548 * 0.785398, 0.546274 * 0.785398);
  
  float base[9];
  normal.x = -normal.x;
  normal.z = -normal.z;
  base[0] = coeff[0] * 1.0;
  base[1] = coeff[1] * normal.z;
  base[2] = coeff[2] * normal.y;
  base[3] = coeff[3] * normal.x;
  base[4] = coeff[4] * normal.x * normal.z;
  base[5] = coeff[5] * normal.z * normal.y;
  base[6] = coeff[6] * (3.0 * normal.y * normal.y - 1.0);
  base[7] = coeff[7] * normal.x * normal.y;
  base[8] = coeff[8] * (normal.x * normal.x - normal.z * normal.z);
  
  const float a0 = 3.141593;
  const float a1 = 2.094395;
  const float a2 = 0.785398;
  result += sh[0]*base[0];
  result += sh[1]*base[1];
  result += sh[2]*base[2];
  result += sh[3]*base[3];
  result += sh[4]*base[4];
  result += sh[5]*base[5];
  result += sh[6]*base[6];
  result += sh[7]*base[7];
  result += sh[8]*base[8];
  result /= 9;
  
  return result;
}

void AmbientIBL(out vec3 diffuse, out vec3 specular, samplerCube irradianceMap, samplerCube prefilterMap, sampler2D brdfLUT, vec3 Albedo, vec3 Normal, vec3 viewDir, vec3 F0, 
                float roughness, float metallic, vec3 sh[9])
{
  vec3 F = FresnelSchlickRoughness(max(dot(Normal, viewDir), 0.0), F0, roughness);
  
  vec3 kS = F;
  vec3 kD = 1.0 - kS;
  kD *= 1.0 - metallic;
  
  vec3 irridiance = SphericalHarmonicsIrradiance(sh, Normal);//texture(irradianceMap, Normal).rgb;
  diffuse = kD * irridiance * Albedo;
  
  // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
  const float MAX_REFLECTION_LOD = 5.0;
  vec3 R = reflect(-viewDir, Normal);
  vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
  vec2 brdf  = texture(brdfLUT, vec2(max(dot(Normal, viewDir), 0.0), roughness)).rg;
  specular = prefilteredColor * (F * brdf.x + brdf.y);
}

vec3 DirLightReflectanceEq(vec3 norm, vec3 viewDir, vec3 lightDir, vec3 lightColor, vec3 albedo, float roughness, float metallic, vec3 F0)
{
  // calculate per-light radiance
  vec3 L = normalize(lightDir);
  vec3 H = normalize(viewDir + L);
  
  // Calculate angles between surface normal and various light vectors.
  float NdotL = max(0.0, dot(norm, L));
  float NdotV = max(0.0, dot(norm, viewDir));
  
  // cook-torrance brdf
  float NDF = DistributionGGX(norm, H, roughness);        
  float G   = GeometrySmith(NdotV, NdotL, roughness);      
  vec3 F    = FresnelSchlick(max(dot(H, viewDir), 0.0), F0);
  
  // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
  // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
  // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
  vec3 kD = mix(vec3(1.0)- F, vec3(0.0), metallic);
   
  vec3 numerator    = NDF * G * F;
  float denominator = 4.0 * NdotV * NdotL;
  vec3 specular     = numerator / max(denominator, 0.001);
  
  // outgoing radiance
  return (kD * albedo + specular) * lightColor * NdotL;
  //return (kD * albedo / M_PI + specular) * lightColor * NdotL;
}

vec3 PointLightReflectanceEq(vec3 worldPos, vec3 norm, vec3 viewDir, vec3 lightPos, vec3 lightColor, float lightAtten, vec3 albedo, float roughness, float metallic, vec3 F0)
{
  // calculate per-light radiance
  vec3 L = normalize(lightPos - worldPos);
  vec3 H = normalize(viewDir + L);
  vec3 radiance     = lightColor * lightAtten;
  
  // Calculate angles between surface normal and various light vectors.
  float NdotL = max(0.0, dot(norm, L));
  float NdotV = max(0.0, dot(norm, viewDir));
  
  // cook-torrance brdf
  float NDF = DistributionGGX(norm, H, roughness);        
  float G   = GeometrySmith(NdotV, NdotL, roughness);      
  vec3 F    = FresnelSchlick(max(dot(H, viewDir), 0.0), F0);
  
  // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
  // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
  // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
  //vec3 kS = F;
  //vec3 kD = vec3(1.0) - kS;
  //kD *= 1.0 - metallic;
  vec3 kD = mix(vec3(1.0)- F, vec3(0.0), metallic);
  
  vec3 numerator    = NDF * G * F;
  float denominator = 4.0 * NdotV * NdotL;
  vec3 specular     = numerator / max(denominator, 0.001);
  
  // outgoing radiance
  return (kD * albedo + specular) * radiance * NdotL;
  //return (kD * albedo / M_PI + specular) * radiance * NdotL;
}