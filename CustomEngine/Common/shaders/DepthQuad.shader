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
  layout(location=0) out vec3 FragColor;

  in VS_OUT
  {
    vec2 UV;
  } fs_in;
  
  uniform sampler2D srcTex;

  void main()
  {
    //FragColor.rgb = vec3(1,0,1);
    FragColor.rgb = texture(srcTex, fs_in.UV).rrr;
  }
}