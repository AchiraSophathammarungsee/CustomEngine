Property
{
  samplerCube skyTex = (0)
}
#VERT
{
  layout(location = 0) in vec3 vPosition;

  out vec3 TexCoords;

  layout(std140) uniform Matrices
  {
    mat4 finalMatrix;
    mat4 projMatrix;
    mat4 viewMatrix;
  };

  uniform mat4 skyMatrix;

  void main()
  {
    TexCoords = 0.5 * vPosition;
    gl_Position = projMatrix * mat4(mat3(viewMatrix)) * vec4(vPosition, 1.0);
    gl_Position = gl_Position.xyww;
  }
}
#FRAG
{
  out vec3 color;

  in vec3 TexCoords;

  void main()
  {
    color = texture(property.skyTex, TexCoords).rgb;
  }
}