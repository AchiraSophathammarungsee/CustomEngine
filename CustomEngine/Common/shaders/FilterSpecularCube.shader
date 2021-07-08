Property
{
  
}
#VERT
{
  // Input vertex data, different for all executions of this shader.
  layout(location = 0) in vec3  vPosition;
  layout(location = 1) in vec3 vNormal;
  layout(location = 2) in vec3 vTangent;
  layout(location = 3) in vec3 vBitangent;
  layout(location = 4) in vec2 vUV;
  
  uniform mat4 projection;
  uniform mat4 view;
  
  out vec3 localPos;

  void main()
  {
    localPos = vPosition;
    gl_Position = projection * view * vec4(vPosition, 1.0f );
  }
}
#FRAG
#include ../Common/shaderlib/mathutil.glsl
#include ../Common/shaderlib/pbr.glsl
{
  in vec3 localPos;
  
  uniform samplerCube envMap;
  uniform float roughness;
  const float PI = 3.14159265359;
  
  int cubeMapSize = 1024;
  
  out vec3 color;
  
  void main()
  {
    // the sample direction equals the hemisphere's orientation 
    vec3 N = normalize(localPos);
    vec3 R = N;
    vec3 V = R;
    
    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;   
    vec3 prefilteredColor = vec3(0.0);
    
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
      vec2 Xi = Hammersley(i, SAMPLE_COUNT);
      vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
      vec3 L  = normalize(2.0 * dot(V, H) * H - V);

      float NdotL = max(dot(N, L), 0.0);
      float NdotH = max(dot(N, H), 0.0);
      float HdotV = max(dot(V, H), 0.0);
      if(NdotL > 0.0)
      {
        // reduce Bright dots in the pre-filter convolution by sampling from mipmap instead
        // find appropriate mipmap to sample
        float mipLevel = 0.0;
        {
          float D = DistributionGGX(N, H, roughness);
          float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001;
          
          float resolution = cubeMapSize; // resolution of source cubemap (per face)
          float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
          float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
          
          mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
        }
        
        prefilteredColor += textureLod(envMap, L, mipLevel).rgb * NdotL;
        totalWeight      += NdotL;
      }
    }
    prefilteredColor = prefilteredColor / totalWeight;
    
    color = prefilteredColor;
  }
}