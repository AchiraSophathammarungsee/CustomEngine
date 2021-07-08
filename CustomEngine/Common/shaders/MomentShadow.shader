Property
{
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

  out vec4 position;
  
  uniform mat4 lightSpaceMatrix;

  void main()
  {
    mat4 boneTransform = GetBoneTransform(vBoneIDs, vWeights);
    gl_Position = lightSpaceMatrix * modelMatrix * boneTransform * vec4(vPosition, 1.0f);
    
    position = gl_Position;
  }
}

#FRAG
#include ../Common/shaderlib/common.glsl
{
  layout(location=0) out vec4 outMoments;
  
  in vec4 position;
  
  float cutoff = 0.5;
  
  const mat4 matr = mat4(-2.07224649,    13.7948857237,  0.105877704,   9.7924062118,
                         32.23703778,  -59.4683975703, -1.9077466311, -33.7652110555,
                         -68.571074599,  82.0359750338,  9.3496555107,  47.9456096605,
                         39.3703274134,-35.364903257,  -6.6543490743, -23.9728048165);
  
  void main()
  {
    // discard fragment if alpha is below cutoff
    /* float alpha = 1;
    if(property.haveDiffMap) alpha = texture(property.diffMap, UV).a;
    if(alpha <= cutoff)
    {
      discard;
    } */
    float depth = position.z / position.w;
    depth = depth * 0.5 + 0.5; //Don't forget to move away from unit cube ([-1,1]) to [0,1] coordinate system
    float z1 = depth;
    float z2 = z1 * depth;
    float z3 = z2 * depth;
    float z4 = z3 * depth;
    outMoments = vec4(z1,z2,z3,z4);
    
    // SHADOW MAP OPTIMIZED MOMENT QUANTIZATION USE TRANSPOSE MATRIX
    outMoments = matr * outMoments;
    outMoments.r += 0.0359558848;
  }
}