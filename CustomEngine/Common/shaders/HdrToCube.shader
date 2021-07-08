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
{
  in vec3 localPos;
  
  uniform sampler2D hdrTex;
  
  out vec3 color;
  
  const vec2 invAtan = vec2(0.1591, 0.3183);
  vec2 SampleSphericalMap(vec3 v)
  {
      vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
      uv *= invAtan;
      uv += 0.5;
      return uv;
  }
  
  void main()
  {
    vec2 uv = SampleSphericalMap(normalize(localPos)); // make sure to normalize localPos
    color = texture(hdrTex, vec2(uv.x, 1.0 - uv.y)).rgb;
  }
}