uniform mat4 bones[64];

mat4 GetBoneTransform(vec4 boneID, vec4 weights)
{
  mat4 boneTransform = mat4(1.0);
  if (weights.x != 0.0)
  {
    boneTransform = bones[int(boneID.x)] * weights.x +
                    bones[int(boneID.y)] * weights.y +
                    bones[int(boneID.z)] * weights.z +
                    bones[int(boneID.w)] * weights.w;
  }
  return boneTransform;
}