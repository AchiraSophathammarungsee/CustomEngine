Property
{
}

#VERT
{
  // Input vertex data, different for all executions of this shader.
  layout(location = 0) in vec3 vPosition;
  layout(location = 4) in vec2 vUV;

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
  in vec2 UV;
  
  uniform sampler2D gPos;
  uniform sampler2D gNorm;
  uniform sampler2D noise;
  
  uniform vec3 samples[64];
  uniform float radius;
  uniform float bias;
  uniform float power;
  
  const int kernelSize = 64;
  
  out float SSAO;

  void main()
  {
    vec3 pos = texture(gPos, UV).rgb;
    pos = vec3(viewMatrix * vec4(pos, 1.0));
    vec3 normal = normalize(vec3(viewMatrix * vec4(texture(gNorm, UV).rgb, 0.0)));
    
    // tile noise texture over screen based on screen dimensions divided by noise size
    vec2 noiseScale = screenParam / 4.0;
    vec3 randomVec = normalize(texture(noise, UV * noiseScale).rgb);
    
    // Using a process called the Gramm-Schmidt process we create an orthogonal basis, 
    // each time slightly tilted based on the value of randomVec. 
    // Note that because we use a random vector for constructing the tangent vector, 
    // there is no need to have the TBN matrix exactly aligned to the geometry's surface, 
    // thus no need for per-vertex tangent (and bitangent) vectors.
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);
    
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
      // get sample position
      vec3 samplePos = TBN * samples[i]; // from tangent to view-space
      samplePos = pos + samplePos * radius; 
      
      // project sample position (to sample texture) (to get position on screen/texture)
      vec4 offset = vec4(samplePos, 1.0);
      offset = projMatrix * offset; // from view to clip-space
      offset.xyz /= offset.w; // perspective divide
      offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
      
      // get sample depth
      float sampleDepth = (viewMatrix * vec4(texture(gPos, offset.xy).rgb, 1.0)).z; // get depth value of kernel sample
      
      // range check & accumulate
      float rangeCheck = smoothstep(0.0, 1.0, radius / abs(pos.z - sampleDepth));
      occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    SSAO = pow(occlusion, power);
  }
}