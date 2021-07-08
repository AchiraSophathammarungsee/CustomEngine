Property
{
  color3 albedo = (0.8, 0.8, 0.8)
  color3 F0 = (0.04, 0.04, 0.04)
  float metallic = (0.0)
  float roughness = (1.0)
  float ao = (0.0)
  sampler2D diffMap = (1)
  sampler2D normMap = (2)
  sampler2D RMAMap = (3)
  bool haveNormMap = (false)
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
    
    vec4 worldNorm = modelMatrix * boneTransform * vec4(vNormal, 0.0f);
    vec4 worldTang = modelMatrix * boneTransform * vec4(cross(vBitangent, vNormal), 0.0f);
    vec4 worldBitang = modelMatrix * boneTransform * vec4(vBitangent, 0.0f);

    vs_out.posWS = worldPos.xyz;
    vs_out.UV = vUV;

    vec3 T = normalize(vec3(worldTang));
    vec3 B = normalize(vec3(worldBitang));
    vec3 N = normalize(vec3(worldNorm));
    vs_out.TBN = (mat3(T, B, N));
    //vs_out.TBN = GetTBN(boneTransform, vNormal, vTangent, vBitangent);

    gl_Position = finalMatrix * worldPos;
  }
}

#FRAG
#include ../Common/shaderlib/common.glsl
{
  layout (location = 0) out vec3 gPosition;
  layout (location = 1) out vec3 gNormal;
  layout (location = 2) out vec4 gDiffuse;
  layout (location = 3) out vec4 gRMA; // roughtness, metallic, AO
  layout (location = 4) out vec3 gF0;

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
      vec3 norm = texture(property.normMap, fs_in.UV).rgb;
      norm = normalize((norm * 2.0) - 1.0);
      gNormal = normalize(fs_in.TBN * norm);
      //gNormal = NormalMapping(texture(property.normMap, fs_in.UV).rgb, fs_in.TBN);
    }
    else
    {
      gNormal = fs_in.TBN[2];
    }

    gDiffuse.rgb = property.albedo * texture(property.diffMap, fs_in.UV).rgb;
    gDiffuse.a = 1.0;
    
    vec3 rma = texture(property.RMAMap, fs_in.UV).rgb;
    gRMA.r = property.roughness * rma.r;
    gRMA.g = property.metallic * rma.g;
    gRMA.b = property.ao * rma.b, 
    gRMA.a = 1.0;
    
    // f0 = mix(f0, albedo, metallic)
    gF0 = mix(property.F0, gDiffuse.rgb, gRMA.g);
  }
}