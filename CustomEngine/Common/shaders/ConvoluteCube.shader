Property
{
  
}
#VERT
{
  // Input vertex data, different for all executions of this shader.
  layout(location = 0) in vec3  vPosition;
  layout(location = 1) in vec3 vNormal;
  layout(location = 2) in vec3 vTangent;
  layout(location = 3) in vec3 vBitangent;
  layout(location = 4) in vec2 vUV;
  
  uniform mat4 projection;
  uniform mat4 view;
  
  out vec3 localPos;

  void main()
  {
    localPos = vPosition;
    gl_Position = projection * view * vec4(vPosition, 1.0f );
  }
}
#FRAG
{
  in vec3 localPos;
  
  uniform samplerCube envMap;
  const float PI = 3.14159265359;
  
  out vec3 color;
  
  void main()
  {
    // the sample direction equals the hemisphere's orientation 
    vec3 normal = normalize(localPos);
  
    vec3 irradiance = vec3(0.0);
  
    // convolution code
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, normal);
    up         = cross(normal, right);
    
    float sampleDelta = 0.025;
    float nrSamples = 0.0; 
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

            irradiance += texture(envMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
  
    color = irradiance;
  }
}