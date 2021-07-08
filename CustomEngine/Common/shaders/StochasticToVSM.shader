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
  layout(location=0) out vec3 MeanTex;
  layout(location=1) out vec3 VarianceTex;

  in VS_OUT
  {
    vec2 UV;
  } fs_in;
  
  uniform sampler2D opaqueTex;
  uniform sampler2D stochastTex;
  uniform float opaqueSize;
  uniform float stochastSizeInv;
  uniform vec2 lightNearFar;
  
  /* 
   Converts a depth buffer value to a camera-space Z value.

   \param clipInfo = (z_f == -inf()) ? Vector3(z_n, -1.0f, 1.0f) : Vector3(z_n * z_f,  z_n - z_f,  z_f);
   \sa G3D::Projection::reconstructFromDepthClipInfo
  */
  float reconstructCSZ(float d) 
  {
    return (lightNearFar.x * lightNearFar.y) / ((lightNearFar.x - lightNearFar.y) * d + lightNearFar.y);
  }

  void main()
  {
    float z = (texture(opaqueTex, fs_in.UV).r);

    // Take the closer of the opaque and transparent
    vec3 transpanrentTex = texture(stochastTex, fs_in.UV).rgb;
    vec3 result;
    result.r = min(z, (transpanrentTex.r));
    result.g = min(z, (transpanrentTex.g));
    result.b = min(z, (transpanrentTex.b));

    // z is negative and z^2 is positive, but the z value will end up being squared before
    // it is applied anyway.
    MeanTex.rgb = result;
    
    // prevent shadow ance
    float dx = dFdx(result.r);
    float dy = dFdy(result.r);
    VarianceTex.r = result.r * result.r + 0.25 * (dx * dx + dy * dy);
    dx = dFdx(result.g);
    dy = dFdy(result.g);
    VarianceTex.g = result.g * result.g + 0.25 * (dx * dx + dy * dy);
    dx = dFdx(result.b);
    dy = dFdy(result.b);
    VarianceTex.b = result.b * result.b + 0.25 * (dx * dx + dy * dy);
    //VarianceTex.r = result.r * result.r;
    //VarianceTex.g = result.g * result.g;
    //VarianceTex.b = result.b * result.b;
  }
}