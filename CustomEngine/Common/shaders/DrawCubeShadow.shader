Property
{
  
}
#VERT
#include ../Common/shaderlib/common.glsl
#include ../Common/shaderlib/bones.glsl
{
  // Input vertex data, different for all executions of this shader.
  layout(location = 0) in vec3  vPosition;
  layout(location = 5) in vec4 vBoneIDs;
  layout(location = 6) in vec4 vWeights;

  void main()
  {
    mat4 boneTransform = GetBoneTransform(vBoneIDs, vWeights);
    gl_Position = modelMatrix * boneTransform * vec4(vPosition, 1.0f );
  }
}
#GEO
{
  layout (triangles) in;
  layout (triangle_strip, max_vertices=18) out;
  
  uniform mat4 shadowMatrices[6];

  out vec4 FragPos; // FragPos from GS (output per emitvertex)

  void main()
  {
    for(int face = 0; face < 6; ++face)
    {
      gl_Layer = face; // built-in variable that specifies to which face we render.
      for(int i = 0; i < 3; ++i) // for each triangle vertex
      {
        FragPos = gl_in[i].gl_Position;
        gl_Position = shadowMatrices[face] * FragPos;
        EmitVertex();
      }    
      EndPrimitive();
    }
  }
}
#FRAG
{
  in vec4 FragPos;

  uniform vec3 lightPos;
  uniform float far_plane;
  
  out vec4 color;
  
  void main()
  {
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // prevent shadow ance
    float dx = dFdx(lightDistance);
    float dy = dFdy(lightDistance);
    float moment2 = lightDistance * lightDistance + 0.25 * (dx * dx + dy * dy);
    
    // write this as modified depth
    color = vec4(lightDistance, moment2, 0.0, 1.0);
  }
}