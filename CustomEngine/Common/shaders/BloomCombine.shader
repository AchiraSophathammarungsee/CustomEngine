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
  
  uniform sampler2D bloom;
  uniform float bloomInten;
  
  out vec3 color;

  void main()
  {
    //vec3 src = texture(source, UV).rgb;
    vec3 bloomCol = texture(bloom, UV).rgb;
    //src += bloomCol * bloomInten;
    color = bloomCol * bloomInten;
    
    // tone mapping
    //color = src / (src + vec3(1));
    // gamma correction
    //const float gamma = 2.2;
    //color = pow(color, vec3(1.0 / gamma));
  }
}