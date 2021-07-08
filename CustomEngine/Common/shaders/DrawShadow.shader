Property
{

}
#VERT
#include ../Common/shaderlib/common.glsl
#include ../Common/shaderlib/bones.glsl
{
  // Input vertex data, different for all executions of this shader.
  layout(location = 0) in vec3  vPosition;
  layout(location = 5) in vec4 vBoneIDs;
  layout(location = 6) in vec4 vWeights;
  
  uniform mat4 lightSpaceMatrix;

  void main()
  {
    mat4 boneTransform = GetBoneTransform(vBoneIDs, vWeights);
    gl_Position = lightSpaceMatrix * modelMatrix * boneTransform * vec4(vPosition, 1.0f );
  }
}
#FRAG
{
  void main()
  {
    // gl_FragDepth = gl_FragCoord.z;
  }
}