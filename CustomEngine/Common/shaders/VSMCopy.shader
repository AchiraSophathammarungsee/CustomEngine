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
{
  layout(location=0) out vec3 MeanTex;
  layout(location=1) out vec3 VarianceTex;

  in vec2 UV;
  
  uniform sampler2D mean;
  uniform sampler2D variance;

  void main()
  {
    MeanTex = texture(mean, UV).rgb;
    VarianceTex = texture(variance, UV).rgb;
  }
}