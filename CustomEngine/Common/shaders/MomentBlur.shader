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
  
  uniform sampler2DArray texArray;
  uniform float layer;
  uniform vec2 blurScale;
  
  layout(location=0) out vec4 Moments;
  
  void GaussianLinear13x13(vec2 uv, out vec4 sum)
  {
    vec2 off1 = vec2(1.4091998770852122517874022103911) * blurScale;
    vec2 off2 = vec2(3.2979348079914822508827200123103) * blurScale;
    vec2 off3 = vec2(5.2062900776825963903107721550453) * blurScale;
    
    sum = texture(texArray, vec3(uv, layer)).rgba * 0.1976406528809569;
    
    sum += texture(texArray, vec3(uv + off1, layer)).rgba * 0.29598550560065581;
    sum += texture(texArray, vec3(uv - off1, layer)).rgba * 0.29598550560065581;
    sum += texture(texArray, vec3(uv + off2, layer)).rgba * 0.0935333619980594;
    sum += texture(texArray, vec3(uv - off2, layer)).rgba * 0.0935333619980594;
    sum += texture(texArray, vec3(uv + off3, layer)).rgba * 0.0116608059608062895;
    sum += texture(texArray, vec3(uv - off3, layer)).rgba * 0.0116608059608062895;
  }

  void main()
  { 
    vec4 sum;
    GaussianLinear13x13(UV, sum);
    
    Moments = sum;
  }
}