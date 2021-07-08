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
  
  uniform sampler2D src;
  uniform vec2 blurScale;
  
  out vec3 color;

  void main()
  {
    // linear filtering
    // weight(i) and weight(i+1) = weight(i) + weight(i+1)
    // offset formula: offset(i) = (i * weight(i) + (i+1) * weight(i+1)) / (weight(i) + weight(i+1))
    // i += 2
    vec2 off1 = vec2(1.4091998770852122129302942439606) * blurScale;
    vec2 off2 = vec2(3.2979348079914822177324248973491) * blurScale;
    
    color = texture(src, UV).rgb * 0.2023600146101466;
    color += texture(src, UV + off1).rgb * 0.3030531945966437;
    color += texture(src, UV - off1).rgb * 0.3030531945966437;
    color += texture(src, UV + off2).rgb * 0.095766798098283;
    color += texture(src, UV - off2).rgb * 0.095766798098283;
  }
}