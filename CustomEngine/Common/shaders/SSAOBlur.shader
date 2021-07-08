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
  in vec2 UV;
  
  uniform sampler2D ssao;
  
  out float SSAO;

  void main()
  {
    vec2 texelSize = 1.0 / vec2(textureSize(ssao, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssao, UV + offset).r;
        }
    }
    
    SSAO = result / (4.0 * 4.0);
  }
}