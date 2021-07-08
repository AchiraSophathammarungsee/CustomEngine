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
  
  uniform sampler2D source;
  uniform float exposure;
  
  out vec3 color;

  void main()
  {
    vec3 hdr = texture(source, UV).rgb;
    
    // HDR tonemap 
    color = vec3(1.0) - exp(-hdr * exposure);
    //color = color / (color + vec3(1.0));
    // gamma correct
    //color = pow(color, vec3(1.0/2.2));
  }
}