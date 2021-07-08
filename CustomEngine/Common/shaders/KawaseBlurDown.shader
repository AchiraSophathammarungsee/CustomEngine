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
  
  uniform sampler2D src;
  uniform vec2 offset;
  uniform vec2 halfPixel;
  
  out vec3 color;

  void main()
  {
    color = 4.0 * texture(src, UV).rgb + 
            texture(src, UV - halfPixel.xy * offset).rgb + 
            texture(src, UV + halfPixel.xy * offset).rgb +
            texture(src, UV + vec2(halfPixel.x, -halfPixel.y) * offset).rgb +
            texture(src, UV - vec2(halfPixel.x, -halfPixel.y) * offset).rgb;
            
    color /= 8.0;
  }
}