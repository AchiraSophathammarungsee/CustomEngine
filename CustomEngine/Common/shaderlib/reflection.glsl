vec3 ReflectCubemap(samplerCube cubeMap, vec3 pos, vec3 normal, vec3 cameraPos)
{
  vec3 i = normalize(pos - cameraPos);
  vec3 r = reflect(i, normal);
  return texture(cubeMap, r).rgb;
}