Property
{
}

#VERT
{
  // Input vertex data, different for all executions of this shader.
  layout(location = 0) in vec3 vPosition;
  layout(location = 4) in vec2 vUV;

  out VS_OUT
  {
    vec2 UV;
  } vs_out;

  void main()
  {
    vs_out.UV = vUV;
    gl_Position = vec4(2.f * vPosition, 1.0f);
  }
}

#FRAG
{
  layout(location=0) out vec4 FragColor;

  in VS_OUT
  {
    vec2 UV;
  } fs_in;
  
  uniform sampler2D srcTex;

  void main()
  {
    FragColor.rgb = texture(srcTex, fs_in.UV).rgb;
  }
}