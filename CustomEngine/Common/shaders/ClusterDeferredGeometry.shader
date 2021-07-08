Property
{
  color3 diffuse = (0.8, 0.8, 0.8)
  color3 specular = (1, 1, 1)
  float shininess = (32.0)
  bool haveDiffMap = (false)
  bool haveNormMap = (false)
  bool haveSpecMap = (false)
  sampler2D diffMap = (1)
  sampler2D normMap = (2)
  sampler2D specMap = (3)
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
    vec2 UV;
    mat3 TBN;
  } vs_out;

  void main()
  {
    mat4 boneTransform = GetBoneTransform(vBoneIDs, vWeights);

    vec4 worldPos = modelMatrix * boneTransform * vec4(vPosition, 1.0f);
    vec4 worldNorm = modelMatrix * boneTransform * vec4(vNormal, 1.0f);
    vec4 worldTang = modelMatrix * boneTransform * vec4(vTangent, 1.0f);
    vec4 worldBitang = modelMatrix * boneTransform * vec4(vBitangent, 1.0f);

    vs_out.posWS = worldPos.xyz;
    vs_out.UV = vUV;

    vec3 T = normalize(worldTang.xyz);
    vec3 B = normalize(worldBitang.xyz);
    vec3 N = normalize(worldNorm.xyz);
    vs_out.TBN = mat3(T, B, N);

    gl_Position = finalMatrix * worldPos;
  }
}

#FRAG
#include ../Common/shaderlib/common.glsl
{
  layout (location = 0) out vec3 gPosition;
  layout (location = 1) out vec3 gNormal;
  layout (location = 2) out vec4 gDiffuse;
  layout (location = 3) out vec4 gSpec;

  in VS_OUT
  {
    vec3 posWS;
    vec2 UV;
    mat3 TBN;
  } fs_in;

  void main()
  {
    gPosition = fs_in.posWS;
    
    if (property.haveNormMap)
    {
      gNormal = texture(property.normMap, fs_in.UV).rgb;
      gNormal = normalize(fs_in.TBN * gNormal);
    }
    else
    {
      gNormal = fs_in.TBN[2];
    }

    gDiffuse.rgb = property.diffuse;
    if (property.haveDiffMap)
      gDiffuse.rgb *= texture(property.diffMap, fs_in.UV).rgb;
    gDiffuse.a = 1.0;

    gSpec.rgb = property.specular;
    if(property.haveSpecMap)
      gSpec.rgb *= texture(property.specMap, fs_in.UV).r;
      
    gSpec.a = property.shininess;
  }
}