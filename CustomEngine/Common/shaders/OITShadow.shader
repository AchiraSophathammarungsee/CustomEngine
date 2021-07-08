Property
{
  float alpha = (0.5)
  color3 diffuse = (0.8, 0.8, 0.8)
  bool haveDiffMap = (false)
  sampler2D diffMap = (1)
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
    vec2 UV;
  } vs_out;

  void main()
  {
    mat4 boneTransform = GetBoneTransform(vBoneIDs, vWeights);
    gl_Position = finalMatrix * modelMatrix * boneTransform * vec4(vPosition, 1.0f);
    
    vs_out.UV = vUV;
  }
}

#FRAG
#include ../Common/shaderlib/common.glsl
{
  layout(location=0) out vec4 oFragData0;
  layout(location=1) out vec4 oFragData1;
  layout(location=2) out vec3 _modulate;
  
  in VS_OUT
  {
    vec2 UV;
  } fs_in;
  
  void main()
  {
    vec4 color = vec4(property.diffuse, property.alpha);
    
    if(haveDiffMap) color.rgb *= texture(property.diffMap, fs_in.uv).rgb;
    
    float weight = max(min(1.0, max(max(color.r, color.g), color.b) * color.a), color.a) *
                   clamp(0.03 / (1e-5 + pow(linearDepth(gl_FragCoord.z) / 200, 4.0)), 1e-2, 3e3);
                   
    oFragData0 = vec4(color.rgb * color.a, color.a) * weight;
    oFragData1.r = color.a;
  }
}