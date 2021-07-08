Property
{
}

#VERT
{
  // Input vertex data, different for all executions of this shader.
  layout(location = 0) in vec3 vPosition;
  layout(location = 1) in vec3 vNormal;
  layout(location = 2) in vec3 vTangent;
  layout(location = 3) in vec3 vBitangent;
  layout(location = 4) in vec2 vUV;
  layout(location = 5) in vec4 vBoneIDs;
  layout(location = 6) in vec4 vWeights;

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
  uniform float threshold;
  
  out vec3 color;

  void main()
  {
    vec3 tex = texture(source, UV).rgb;
    float brightness = dot(tex, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > threshold)
    {
      color = tex;
    }
    else
    {
      color = vec3(0);
    }
  }
}